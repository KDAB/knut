#ifndef DOCUMENTLOADER_H
#define DOCUMENTLOADER_H

#include <QJsonObject>

enum FileType
{
    RcFile,
    JsonFile,
    Auto
};

QJsonObject loadDocument(const QString &filename,
        const QString &resourceFile = QString(),
        FileType type = Auto, bool useQrc = false);
#endif // DOCUMENTLOADER_H
