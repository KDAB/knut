#pragma once

#include "textdocument.h"

namespace Core {

class QmlDocument : public TextDocument
{
    Q_OBJECT

public:
    explicit QmlDocument(QObject *parent = nullptr);
};

}
