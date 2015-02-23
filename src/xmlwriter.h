#ifndef XMLCONVERTER_H
#define XMLCONVERTER_H

#include <QByteArray>

class QJsonObject;

QByteArray dialogToUi(const QJsonObject &dialog);

#endif // XMLCONVERTER_H
