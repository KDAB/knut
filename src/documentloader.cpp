#include "documentloader.h"
#include "document.h"
#include "rcfileparser.h"
#include "converter.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QFileInfo>
#include <QFile>
#include <QLoggingCategory>


Q_LOGGING_CATEGORY(loader, "loader")

static QJsonObject loadJsonFile(const QString &jsonFile)
{
    QFile file(jsonFile);

    if (file.open(QIODevice::ReadOnly)) {
        auto jsonDoc = QJsonDocument::fromBinaryData(file.readAll());

        if (!jsonDoc.isNull())
            return jsonDoc.object();
    }

    qCCritical(loader) << "Can't load " << jsonFile;
    return QJsonObject();
}

static QJsonObject normalizeRc(const QJsonObject &root,
        const QString &resourceFile)
{
    QJsonObject o = root;
    QJsonObject dialogs = convertDialogs(documentDialogs(o));

    documentSetDialogs(o, dialogs);

    return o;
}

static QJsonObject loadRcFile(const QString &rcFile,
                        const QString &resourceFile)
{
    auto root = parseRcFile(rcFile);
    return normalizeRc(root, resourceFile);
}

QJsonObject loadDocument(const QString &filename,
        const QString &resourceFile, FileType type)
{
    if (type == Auto) {
        QFileInfo fi(filename);
        type = (fi.completeSuffix() == "json") ? JsonFile : RcFile;
    }

    QJsonObject document;

    switch (type) {
    case RcFile:
        return loadRcFile(filename, resourceFile);
    case JsonFile:
        return loadJsonFile(filename);
    default:
        Q_ASSERT(false);
        break;
    }

    return QJsonObject();
}
