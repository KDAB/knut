#ifndef XMLCONVERTER_H
#define XMLCONVERTER_H

#include <QByteArray>

class QJsonObject;

QByteArray dialogToUi(const QJsonObject &dialog);
QByteArray assetsToQrc(const QJsonObject &assets);

#endif // XMLCONVERTER_H
