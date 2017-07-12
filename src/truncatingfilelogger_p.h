#include <QtDebug>

void init_global_logging(bool enable_debug);
void global_message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
