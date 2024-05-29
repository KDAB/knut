/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QString>
#include <QStringList>
#include <memory>
#include <vector>

struct Data
{
    struct Notification
    {
        QString method;
        QString params;
    };
    struct Request
    {
        QString method;
        QString params;
        QString result;
        QString error = "std::nullptr_t";
    };
    struct Enumeration
    {
        struct Value
        {
            QString name;
            QString comment;
            QString value;
        };

        QString name;
        QString comment;
        bool isString = false;
        std::vector<Value> values;
    };
    struct Type
    {
        QString name;
        QString comment;
        QString dataType;
        QStringList dependencies;
    };

    struct Interface
    {
        struct Property
        {
            QString name;
            QString comment;
            QString dataType;
        };
        QString name;
        QString comment;
        QStringList extends;
        std::vector<Property> properties;
        std::vector<Interface> children;
        QStringList dependencies;
    };

    std::vector<Notification> notifications;
    std::vector<Request> requests;
    std::vector<Enumeration> enumerations;
    std::vector<Type> types;
    std::vector<Interface> interfaces;
};

struct MetaData
{
    struct Type;
    struct Interface;

    using TypePtr = std::shared_ptr<Type>;
    using InterfacePtr = std::shared_ptr<Interface>;

    enum class TypeKind {
        Base,
        Reference,
        Array,
        Map,
        And,
        Or,
        Tuple,
        Literal,
        StringLiteral,
        IntegerLiteral,
        BooleanLiteral
    };

    struct Common
    {
        QString name;
        QString documentation;
        QString since;

        explicit Common(QString _name = QLatin1String(""))
            : name(std::move(_name))
        {
        }

        bool is_deprecated() const { return documentation.contains("@deprecated"); }
    };

    struct Type : public Common
    {
        QStringList dependencies;
        QString value = "std::nullptr_t";

        TypeKind kind = TypeKind::Base;
        std::vector<TypePtr> items;

        using Common::Common;

        virtual ~Type() = default;
        virtual bool is_interface() const { return false; }
    };

    struct Notification : public Common
    {
        TypePtr params;
    };

    struct Request : public Common
    {
        TypePtr params;
        TypePtr result;
        TypePtr partialResult;
        QString error = "std::nullptr_t";
    };

    struct Enumeration : public Common
    {
        enum Type { Invalid, String, Integer, UInteger };

        struct Value : public Common
        {
            QString value;
        };

        std::vector<Value> values;
        Type type = Type::Invalid;
        bool supportsCustomValues = false;
    };

    struct Interface : public Type
    {
        // Use Type::items for properties

        std::vector<TypePtr> extends;
        std::vector<TypePtr> mixins;

        using Type::Type;

        bool is_interface() const override { return true; }
    };

    std::vector<Notification> notifications;
    std::vector<Request> requests;
    std::vector<Enumeration> enumerations;
    std::vector<TypePtr> types;
    std::vector<InterfacePtr> interfaces;
};
