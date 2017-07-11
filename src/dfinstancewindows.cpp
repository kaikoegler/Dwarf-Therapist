/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QTextCodec>

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <psapi.h>

#include "dfinstance.h"
#include "dfinstancewindows.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "dwarf.h"
#include "utils.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "dwarftherapist.h"

#if defined(Q_PROCESSOR_X86_32)
#define BASE_ADDR 0x400000UL
#elif defined(Q_PROCESSOR_X86_64)
#define BASE_ADDR 0x140000000ULL
#else
#error Unsupported architecture
#endif

DFInstanceWindows::DFInstanceWindows(QObject* parent)
    : DFInstance(parent)
    , m_proc(0)
{}

DFInstanceWindows::~DFInstanceWindows() {
    // CloseHandle(0) is a no-op
    CloseHandle(m_proc);
}

static QString handle_error(QString pre_string) {
    DWORD err = GetLastError();
    LPWSTR bufPtr = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, 0, (LPWSTR)&bufPtr, 0, NULL);
    LOGE << pre_string << bufPtr ? QString::fromWCharArray(bufPtr).trimmed()
                                 : QString("Unknown Error %1").arg(err);
    LocalFree(bufPtr);
    return result;
}

QString DFInstanceWindows::calculate_checksum(const IMAGE_NT_HEADERS &pe_header) {
    DWORD compile_timestamp = pe_header.FileHeader.TimeDateStamp;
    LOGI << "Target EXE was compiled at " << QDateTime::fromTime_t(compile_timestamp).toString(Qt::ISODate);
    return hexify(compile_timestamp).toLower();
}

QString DFInstanceWindows::read_string(VPTR addr) {
    size_t len = read_int(addr + memory_layout()->string_length_offset());
    size_t cap = read_int(addr + memory_layout()->string_cap_offset());
    VPTR buffer_addr = addr + memory_layout()->string_buffer_offset();
    if (cap >= 16)
        buffer_addr = read_addr(buffer_addr);

    VPTR buf[1024];

    if (len == 0 || cap == 0) {
        LOGW << "string at" << addr << "is zero-length or zero-cap";
        return "";
    }
    if (len > cap) {
        // probably not really a string
        LOGW << "string at" << addr << "is length" << len << "which is larger than cap" << cap;
        return "";
    }
    if (cap > sizeof(buf)) {
        LOGW << "string at" << addr << "is cap" << cap << "which is suspiciously large, ignoring";
        return "";
    }

    read_raw(buffer_addr, len, buf);
    return QTextCodec::codecForName("IBM437")->toUnicode(buf, len);
}

size_t DFInstanceWindows::write_string(VPTR addr, const QString &str) {
    /*

      THIS IS TOTALLY DANGEROUS

      */

    // TODO, don't write strings longer than 15 characters to the string
    // unless it has already been expanded to a bigger allocation

    int cap = read_int(addr + memory_layout()->string_cap_offset());
    VPTR buffer_addr = addr + memory_layout()->string_buffer_offset();
    if( cap >= 16 )
        buffer_addr = read_addr(buffer_addr);

    int len = qMin<int>(str.length(), cap);
    write_int(addr + memory_layout()->string_length_offset(), len);

    QByteArray data = QTextCodec::codecForName("IBM 437")->fromUnicode(str);
    int bytes_written = write_raw(buffer_addr, len, data.data());
    return bytes_written;
}

bool DFInstanceWindows::mmap(size_t size) {
    m_alloc_start = VirtualAllocEx(m_proc, NULL, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!m_alloc_start)
        handle_error("remote VirtualAllocEx map failed:");
    return m_alloc_start;
}

bool DFInstanceWindows::mremap(size_t new_size) {
    m_alloc_start = VirtualAllocEx(m_proc, m_alloc_start, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!m_alloc_start)
        handle_error("remote VirtualAllocEx remap failed:");
    return m_alloc_start;
}

size_t DFInstanceWindows::read_raw(VPTR addr, size_t bytes, void *buffer) {
    ZeroMemory(buffer, bytes);
    size_t bytes_read = 0;
    if (!ReadProcessMemory(m_proc, reinterpret_cast<LPCVOID>(addr), buffer, bytes, &bytes_read))
        handle_error("ReadProcessMemory failed:");
    return bytes_read;
}

size_t DFInstanceWindows::write_raw(VPTR addr, size_t bytes, const void *buffer) {
    size_t bytes_written = 0;
    if (!WriteProcessMemory(m_proc, reinterpret_cast<LPVOID>(addr), buffer, bytes, &bytes_written))
        handle_error("WriteProcessMemory failed:");

    return bytes_written;
}

static const QSet<QString> df_window_classes{"OpenGL", "SDL_app"};

BOOL CALLBACK static enumWindowsProc(HWND hWnd, LPARAM lParam) {
    auto pids = reinterpret_cast<QSet<PID> *>(lParam);
    WCHAR classNameW[8];
    if (!GetClassNameW(hWnd, classNameW, sizeof(classNameW))) {
        handle_error("GetClassName failed:");
        return false;
    }

    if (!className && wcscmp(className, L"OpenGL") && wcscmp(className, L"SDL_app"))
        return true;

    WCHAR windowName[16] = {0};
    if (!GetWindowTextW(hWnd, windowName, sizeof(windowName))) {
        handle_error("GetWindowText failed:");
        return false;
    }

    Q_ASSERT(windowName);

    if (wcscmp(windowName, L"Dwarf Fortress"))
        return true;

    PID pid;

    GetWindowThreadProcessId(hWnd, &pid);
    if (!pid) {
        LOGE << "could not get PID for hwnd";
        return false;
    }

    pids << pid;

    return true;
}

bool DFInstanceWindows::set_pid(){
    QSet<PID> pids;
    if (!EnumWindows(enumWindowsProc, static_cast<LPARAM>(&pids))) {
        LOGE << "error enumerating windows";
        return false;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        handle_error("error creating toolhelp32 process snapshot:");
        return false;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);

    if (!Process32First(snapshot, &pe32))
        return false;

    do {
        if (!_tcscmp(&pe32.szExeFile, _T("Dwarf Fortress.exe")))
            pids << pe32.th32ProcessID;
    } while (Process32Next(snapshot, &pe32));

    m_pid = select_pid(pids);

    return m_pid != 0;
}

bool DFInstanceWindows::df_running(){
    DWORD cur_pid = m_pid;
    return (set_pid() && cur_pid == m_pid);
}

void DFInstanceWindows::find_running_copy() {
    m_status = DFS_DISCONNECTED;
    LOGI << "attempting to find running copy of DF";

    if(!set_pid()){
        return;
    }

    LOGI << "PID of process is: " << m_pid;

    m_proc = OpenProcess(PROCESS_QUERY_INFORMATION
                         | PROCESS_VM_OPERATION
                         | PROCESS_VM_READ
                         | PROCESS_VM_WRITE, false, m_pid);
    if (!m_proc) {
        handle_error("Error opening process:");
    }
    LOGI << "PROC HANDLE:" << m_proc;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        handle_error("Error creating toolhelp32 snapshot:");
        return;
    } else {
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32);
        if (!Module32First(snapshot, &me32)) {
            handle_error("Error enumerating modules:");
            return;
        } else {
            VPTR base_addr = (VPTR )me32.modBaseAddr;
            IMAGE_DOS_HEADER dos_header;
            read_raw(base_addr, sizeof(dos_header), &dos_header);
            if(dos_header.e_magic != IMAGE_DOS_SIGNATURE){
                qWarning() << "invalid executable";
                return;
            }

            //the dos stub contains a relative address to the pe header, which is used to get the pe header information
            IMAGE_NT_HEADERS pe_header;
            read_raw(base_addr + dos_header.e_lfanew, sizeof(pe_header), &pe_header);
            if(pe_header.Signature != IMAGE_NT_SIGNATURE){
                qWarning() << "unsupported PE header type";
                return;
            }

            LOGI << "RAW BASE ADDRESS:" << base_addr;
            m_base_addr = base_addr - pe_header.OptionalHeader.ImageBase;

            m_status = DFS_CONNECTED;
            set_memory_layout(calculate_checksum(pe_header));

        }
        CloseHandle(snapshot);     // Must clean up the snapshot object!
    }

    WCHAR modName[MAX_PATH];
    DWORD lenModName = GetModuleFileNameExW(m_proc, NULL, modName, MAX_PATH);
    if (lenModName) {
        QString exe_path = QString::fromWCharArray(modName, lenModName);
        LOGI << "GetModuleFileNameEx returned: " << exe_path;
        QFileInfo exe(exe_path);
        m_df_dir = exe.absoluteDir();
        LOGI << "Dwarf Fortress path:" << m_df_dir.absolutePath();
    }

    return;
}
