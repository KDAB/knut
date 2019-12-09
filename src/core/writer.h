#ifndef WRITER_H
#define WRITER_H

#include "converter.h"
#include "global.h"

class QIODevice;

namespace Writer {

void writeUi(QIODevice *device, const Converter::Widget &widget);

void writeQrc(QIODevice *device, const QVector<Converter::Asset> &assets, bool useAlias = true);
}

#endif // WRITER_H
