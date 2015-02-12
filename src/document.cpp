#include "document.h"

#include <QJsonDocument>

namespace {
static const auto AssetsKey = QStringLiteral("assets");
static const auto DialogsKey = QStringLiteral("dialogs");
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

QJsonObject documentDialog(const QJsonObject &root, const QString &id)
{
    auto dialogs = documentDialogs(root);
    return valueForObject(dialogs, id);
}

QJsonObject documentAsset(const QJsonObject &root, const QString &id)
{
    auto assets = documentAssets(root);
    return valueForObject(assets, id);
}

void documentSetDialogs(QJsonObject &root, const QJsonObject &dialogs)
{
    root.insert(DialogsKey, dialogs);
}

QByteArray documentToByteArray(const QJsonObject &o)
{
    return QJsonDocument(o).toJson();
}
