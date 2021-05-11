#pragma once

#include <QString>
#include <QStringList>

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
