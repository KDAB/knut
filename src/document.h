#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QJsonObject>
#include <QByteArray>
#include <QByteArrayList>

QJsonObject documentDialogs(const QJsonObject &root);
QJsonObject documentAssets(const QJsonObject &root);
QJsonObject documentResources(const QJsonObject &root);
QJsonObject documentDialog(const QJsonObject &root, const QString &id);
QString documentAsset(const QJsonObject &root, const QString &id);
QString documentAsset(const QJsonObject &root, int id);

void documentSetDialogs(QJsonObject &root, const QJsonObject &dialogs);

QByteArray documentToByteArray(const QJsonObject &o);

#endif // DOCUMENT_H
