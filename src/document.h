#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QJsonObject>

struct Document
{
    Document() = default;
    Document(const QJsonObject &data);

    // Dialogs
    QVariantMap dialogs;
    QJsonObject dialog(const QString &id);

    // List of assets
    QVariantMap assets;
    QString assetPath(const QString &id);

    bool hasError = true;
};

// Reader
Document readFromRcFile(const QString &rcFile, const QString &resourceFile = QString{});
Document readFomrJsonFile(const QString &jsonFile);

// Writer
QJsonDocument createJsonDocument(const Document &doc);
void writeToJsonFile(const Document &doc, const QString &jsonFile);

#endif // DOCUMENT_H
