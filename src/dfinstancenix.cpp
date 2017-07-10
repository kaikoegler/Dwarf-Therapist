#include "dfinstancenix.h"
#include "truncatingfilelogger.h"
#include <QCryptographicHash>
#include <QFile>
#include <QTextCodec>

struct STLStringHeader {
    size_t length;
    size_t capacity;
    int refcnt;
};

DFInstanceNix::DFInstanceNix(QObject *parent)
    : DFInstance(parent)
    , m_pid(0)
{}

QString DFInstanceNix::calculate_checksum() {
    // ELF binaries don't seem to store a linker timestamp, so just MD5 the file.
    QFile proc(m_loc_of_dfexe);
    QCryptographicHash hash(QCryptographicHash::Md5);
    if (!proc.open(QIODevice::ReadOnly)) {
        LOGE << "FAILED TO READ DF EXECUTABLE:" << m_loc_of_dfexe;
        return QString("UNKNOWN");
    }
#if QT_VERSION >= 0x050000
    hash.addData(&proc);
#else
    char buf[4096];
    qint64 len;
    while ((len = proc.read(buf, sizeof(buf))) > 0) {
        hash.addData(buf, len);
    }
#endif
    QString md5 = hexify(hash.result().mid(0, 4)).toLower();
    TRACE << "GOT MD5:" << md5;
    return md5;
}

QString DFInstanceNix::read_string(VPTR addr) {
    char buf[1024];
    read_raw(read_addr(addr), sizeof(buf), (void *)buf);

    return QTextCodec::codecForName("IBM437")->toUnicode(buf);
}

bool DFInstanceNix::df_running(){
    pid_t cur_pid = m_pid;
    return (set_pid() && cur_pid == m_pid);
}

size_t DFInstanceNix::write_string(VPTR addr, const QString &str) {
    // Ensure this operation is done as one transaction
    attach();
    VPTR buffer_addr = get_string(str);
    if (buffer_addr)
        // This unavoidably leaks the old buffer; our own
        // cannot be deallocated anyway.
        write_raw(addr, sizeof(VPTR), &buffer_addr);
    detach();
    return buffer_addr ? str.length() : 0;
}

VPTR DFInstanceNix::get_string(const QString &str) {
    if (m_string_cache.contains(str))
        return m_string_cache[str];

    QByteArray data = QTextCodec::codecForName("IBM437")->fromUnicode(str);

    STLStringHeader header;
    header.capacity = header.length = data.length();
    header.refcnt = -1; // huge refcnt to avoid dealloc

    QByteArray buf(reinterpret_cast<char *>(&header), sizeof(header));
    buf.append(data);
    buf.append(char(0));

    VPTR addr = alloc_chunk(buf.length());

    if (addr) {
        write_raw(addr, buf.length(), buf.data());
        addr += sizeof(header);
    }

    return m_string_cache[str] = addr;
}
