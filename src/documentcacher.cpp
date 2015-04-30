#include "documentcacher.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QFile>
#include <QDebug>

void cacheDocument(const QString &fileName,
        const QJsonObject &root)
{
    QJsonDocument doc(root);

    QFile f(fileName);

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical()
            << QObject::tr("Cannot open cache file for writing");

        return;
    }

    if (f.write(doc.toJson()) == -1) {
        qCritical()
            << QObject::tr("Error writing cache file: %1").arg(f.errorString());

        return;
    }
}

