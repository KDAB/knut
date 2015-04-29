#ifndef CONVERTER
#define CONVERTER

#include <QJsonObject>

QJsonObject convertDialog(const QJsonObject &dialog);
QJsonObject convertDialogs(const QJsonObject &root, bool useQrc);

#endif // CONVERTER

