#include "messagehandler.h"

#include <QMutex>
#include <QMutexLocker>

#include <fstream>

void messageHandler(QtMsgType type,
        const QMessageLogContext &context,
        const QString &message)
{
    static QMutex mutex;

    QMutexLocker lock(&mutex);

    static std::ofstream logFile("knut.log");

    if (!logFile)
        return;

    logFile << qPrintable(qFormatLogMessage(type,
                context, message)) << std::endl;
}
