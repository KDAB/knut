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

    std::vector<Notification> notifications;
    Notification &currentNotification() { return *(notifications.rbegin()); }
};
