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
QString dialogId(const QJsonObject &dialog);

void documentSetDialogs(QJsonObject &root, const QJsonObject &dialogs);
void documentSetAssets(QJsonObject &root, const QJsonObject &assets);

QByteArray documentToByteArray(const QJsonObject &o);

#endif // DOCUMENT_H
