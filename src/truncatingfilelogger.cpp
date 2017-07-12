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

http://www.opensource.org/licenses/mit-license.php
*/
#include "truncatingfilelogger.h"
#include "truncatingfilelogger_p.h"
#include <QFile>
#include <QDebug>
#include <QString>

static QFile *output;
static bool debug_enabled = true;

#if QT_VERSION < 0x050400
QString qMessageFormatString(QtMsgType type, const QMessageLogContext &context, const QString &str);
#endif

void init_global_logging(bool enable_debug) {
    debug_enabled = enable_debug;
#ifdef Q_OS_WIN
    output = new QFile("DwarfTherapist.log");
    if (!output->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qFatal("Could not open log for writing: %s", output->errorString().toLocal8Bit().data());
    }
#else
    output = new QFile();
    if (!output->open(stdout, QIODevice::WriteOnly | QIODevice::Text)) {
        qFatal("Could not open log for writing: %s", output->errorString().toLocal8Bit().data());
    }
#endif
    qSetMessagePattern("[%{time process}] <%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}> %{message} (%{file}:%{line})");
    qInstallMessageHandler(global_message_handler);
}

void global_message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (!output)
        abort();

    if (type != QtDebugMsg || debug_enabled) {
#if QT_VERSION >= 0x050400
        output->write(qFormatLogMessage(type, context, msg).toLocal8Bit());
#else
        output->write(qMessageFormatString(type, context, msg).toLocal8Bit());
#endif
        output->write("\n");
        output->flush();
    }

    if (type == QtFatalMsg)
        abort();
}
