#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QDebug>

void messageHandler(QtMsgType type, const QMessageLogContext &context,
        const QString &message);

#endif // MESSAGEHANDLER_H
