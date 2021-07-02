#pragma once

#include "textdocument.h"

namespace Core {

class CppDocument : public TextDocument
{
    Q_OBJECT
public:
    explicit CppDocument(QObject *parent = nullptr);
};

} // namespace Core
