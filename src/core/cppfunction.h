#pragma once

#include <QString>
#include <QVector>

#include "textrange.h"

namespace Core {

struct Argument
{
    Q_GADGET

    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(QString name MEMBER name)

public:
    QString type;
    QString name;
};
bool operator==(const Argument &left, const Argument &right);

struct CppFunction
{
    Q_GADGET

    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString returnType MEMBER returnType)
    Q_PROPERTY(QVector<Argument> arguments MEMBER arguments)
    Q_PROPERTY(Core::TextRange range MEMBER range)

public:
    QString name;
    QString returnType;
    QVector<Argument> arguments;
    TextRange range;
};
bool operator==(const CppFunction &left, const CppFunction &right);

} // namespace Core

Q_DECLARE_METATYPE(Core::Argument)
Q_DECLARE_METATYPE(Core::CppFunction)
Q_DECLARE_METATYPE(QVector<Core::Argument>)
