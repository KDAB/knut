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
#include <QTextStream>

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

static QJsonObject normalizeRc(QJsonObject root)
{
    QJsonObject dialogs = convertDialogs(root);
    documentSetDialogs(root, dialogs);
    return root;
}

static QJsonObject loadResourceFile(const QJsonObject &root,
                                    const QString &resourceFile)
{
    QFile file(resourceFile);
    if (!file.open(QIODevice::ReadOnly))
        qCCritical(loader) << "Can't load " << resourceFile;

    auto assets = documentAssets(root);
    QJsonObject resource;
    QTextStream stream(&file);

    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (!line.startsWith(QStringLiteral("#define")))
            continue;

        QStringList fields = line.split(QStringLiteral(" "), QString::SkipEmptyParts);
        if (fields.size() < 3)
            continue;
        if (fields.at(2).isEmpty())
            continue;

        // Check that the value is an asset
        const auto value = fields.at(1);
        if (!assets.contains(value))
            continue;

        // Get the key
        bool ok;
        const int key = fields.at(2).toInt(&ok);
        if (!ok)
            continue;

        // Store the result
        resource.insert(QString::number(key), value);
    }
    return resource;
}

static QJsonObject loadRcFile(const QString &rcFile,
                        const QString &resourceFile)
{
    auto root = parseRcFile(rcFile);
    if (!resourceFile.isEmpty())
        root.insert("resources", loadResourceFile(root, resourceFile));
    return normalizeRc(root);
}

QJsonObject loadDocument(const QString &filename,
        const QString &resourceFile, FileType type)
{
    if (type == Auto) {
        QFileInfo fi(filename);
        type = (fi.completeSuffix() == "json") ? JsonFile : RcFile;
    }

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
