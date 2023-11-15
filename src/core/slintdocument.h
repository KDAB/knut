#pragma once
#include "textdocument.h"

namespace Core {

class SlintDocument : public TextDocument
{
    Q_OBJECT

public:
    explicit SlintDocument(QObject *parent = nullptr);
};

}
