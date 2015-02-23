#include "document.h"

#include <QJsonDocument>

namespace {
static const auto AssetsKey = QStringLiteral("assets");
static const auto DialogsKey = QStringLiteral("dialogs");
static const auto ResourcesKey = QStringLiteral("resources");
}

static inline QJsonObject valueForObject(const QJsonObject &object,
        const QString &key)
{
    return object.value(key).toObject();
}

QJsonObject documentDialogs(const QJsonObject &root)
{
    return valueForObject(root, DialogsKey);
}

QJsonObject documentAssets(const QJsonObject &root)
{
    return valueForObject(root, AssetsKey);
}

QJsonObject documentResources(const QJsonObject &root)
{
    return valueForObject(root, ResourcesKey);
}

QJsonObject documentDialog(const QJsonObject &root, const QString &id)
{
    auto dialogs = documentDialogs(root);
    return valueForObject(dialogs, id);
}

QString documentAsset(const QJsonObject &root, const QString &id)
{
    auto assets = documentAssets(root);
    if (!assets.contains(id))
        return id;
    return assets.value(id).toString();
}

QString documentAsset(const QJsonObject &root, int id)
{
    auto key = QString::number(id);
    if (!root.contains(ResourcesKey))
        return key;
    auto resources = documentResources(root);
    auto assetId = resources.value(key).toString();
    if (assetId.isEmpty())
        return key;
    return documentAsset(root, assetId);
}

void documentSetDialogs(QJsonObject &root, const QJsonObject &dialogs)
{
    root.insert(DialogsKey, dialogs);
}

QByteArray documentToByteArray(const QJsonObject &o)
{
    return QJsonDocument(o).toJson();
}

QString dialogId(const QJsonObject &dialog)
{
    return dialog.value("id").toString();
}
