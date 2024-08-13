/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QString>
#include <QStringList>
#include <vector>

struct Data
{
    enum PositionInGroup {
        FirstInGroup = 1,
        InTheMiddle,
        LastInGroup,
    };

    struct MappedType
    {
        QString typeName;
        QString sourceFile;
        QString docFile;
        QString qmlModule;
        QString group;
        PositionInGroup positionInGroup = InTheMiddle;
    };

    struct Block
    {
        QString brief;
        QString description;
        QString since;
        QStringList seeAlso;
        QString group;
        PositionInGroup positionInGroup = InTheMiddle;
        bool isExperimental = false;
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

    struct SignalBlock : public Block
    {
        QmlMethod method;
        QString qmlType;
    };

    std::vector<TypeBlock> types;
    std::vector<PropertyBlock> properties;
    std::vector<MethodBlock> methods;
    std::vector<SignalBlock> qmlSignals;
    std::vector<MappedType> mappedTypes;
};
