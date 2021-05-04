#pragma once

#include <QString>

#include <vector>

struct Specification
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

    std::vector<Notification> notifications;
    Notification &currentNotification() { return *(notifications.rbegin()); }

    std::vector<Request> requests;
    Request &currentRequest() { return *(requests.rbegin()); }
};
