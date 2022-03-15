#pragma once

#include "symbol.h"

#include <QString>
#include <QVector>

namespace Core {

struct Symbol;

struct CppClass
{
private:
    Q_GADGET

    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QVector<Symbol> members MEMBER members)

public:
    QString name;
    QVector<Symbol> members;
};
bool operator==(const CppClass &left, const CppClass &right);

} // namespace Core

Q_DECLARE_METATYPE(Core::CppClass)
