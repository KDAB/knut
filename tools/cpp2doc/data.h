#pragma once

#include <QString>
#include <QStringList>

#include <vector>

struct Data
{
    struct Block
    {
        QString brief;
        QString description;
        QString since;
        QStringList seeAlso;
    };

    struct TypeBlock : public Block
    {
        QString name;
        QString qmlModule;
        QString inherits;
    };

    struct PropertyBlock : public Block
    {
        QString name;
        QString type;
        QString qmlType;
    };

    struct QmlMethod
    {
        struct Parameter
        {
            QString name;
            QString type;
        };
        QString name;
        QString returnType;
        std::vector<Parameter> parameters;
    };

    struct MethodBlock : public Block
    {
        std::vector<QmlMethod> methods;
        QString qmlType;
    };

    std::vector<TypeBlock> types;
    std::vector<PropertyBlock> properties;
    std::vector<MethodBlock> methods;
};
