#ifndef CONVERTER
#define CONVERTER

#include <QJsonObject>

struct Document;

QJsonObject convertDialog(const Document &doc, QJsonObject dialog);

#endif // CONVERTER

