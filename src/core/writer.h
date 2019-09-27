#ifndef WRITER_H
#define WRITER_H

#include "converter.h"
#include "global.h"

class QIODevice;

namespace Writer {

void writeUi(QIODevice *device, const Converter::Widget &widget);

}

#endif // WRITER_H
