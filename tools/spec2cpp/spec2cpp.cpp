#include "data.h"
#include "specparser.h"
#include "specwriter.h"

#include <QDebug>

// Printing
///////////////////////////////////////////////////////////////////////////////
#if 0
static void printInterface(const Data::Interface &interface, int level = 0)
{
    QString spaces(level * 4, ' ');
    if (interface.extends.isEmpty())
        qDebug().noquote() << QString("%1%2  {").arg(spaces, interface.name);
    else
        qDebug().noquote() << QString("%1%2 extends %3 {").arg(spaces, interface.name, interface.extends.join(','));

    for (const auto &child : interface.children)
        printInterface(child, level + 1);
    for (const auto &property : interface.properties)
        qDebug().noquote() << QString("%1    %2 : %3").arg(spaces, property.name, property.dataType);
    qDebug().noquote().nospace() << spaces << "}";
}

static void printSpecification(const Data &data)
{
    if (1) {
        qDebug().noquote() << "Notifications\n=============";
        for (const auto &notification : data.notifications)
            qDebug().noquote() << QString("%1(%2)").arg(notification.method, notification.params);
        qDebug() << "\n";
    }

    if (1) {
        qDebug().noquote() << "Requests\n========";
        for (const auto &request : data.requests)
            qDebug().noquote() << QString("%1(%2) => %3").arg(request.method, request.params, request.result);
        qDebug() << "\n";
    }

    if (1) {
        qDebug().noquote() << "Enums\n=====";
        for (const auto &enumeration : data.enumerations) {
            qDebug().noquote() << QString("%1(%2) {").arg(enumeration.name).arg(enumeration.isString);
            for (const auto &v : enumeration.values)
                qDebug().noquote() << QString("\t%1 = %2,").arg(v.name, v.value);
            qDebug().noquote() << "}\n";
        }
        qDebug() << "\n";
    }

    if (1) {
        qDebug().noquote() << "Types\n=====";
        for (const auto &type : data.types)
            qDebug().noquote() << QString("%1 = %2\n").arg(type.name, type.dataType);
        qDebug() << "\n";
    }

    if (1) {
        qDebug().noquote() << "Interfaces\n==========";
        for (const auto &interface : data.interfaces)
            printInterface(interface);
        qDebug() << "\n";
    }
}

static void printDependencies(const Data &data)
{
    if (1) {
        qDebug().noquote() << "Types\n=====";
        for (const auto &type : data.types)
            qDebug().noquote() << type.dependencies;
        qDebug() << "\n";
    }

    if (1) {
        qDebug().noquote() << "Interfaces\n==========";
        for (const auto &interface : data.interfaces)
            qDebug().noquote() << interface.dependencies;
        qDebug() << "\n";
    }
}
#endif

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    SpecParser parser;
    Data data = parser.parse(":/specification-3-16.md");
    // printSpecification(data);
    // printDependencies(data);

    SpecWriter writer(data);
    writer.saveNotifications();
    writer.saveRequests();
    writer.saveCode();
}
