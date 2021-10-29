#ifndef SPECPARSER_H
#define SPECPARSER_H

#include "data.h"

#include <vector>

class SpecParser
{
    enum State {
        None,
        InNotification,
        InRequest,
        InCode,
        InComment,
        InEnum,
        InType,
        InInterface,
        InInterfaceProperties,
    };

public:
    SpecParser();

    Data parse(const QString &fileName);

private:
    // States
    State state() const { return *m_states.rbegin(); }
    void pushState(State newState);
    void popState();

    // Utility methods
    void addDependency(const QString &name);
    void addDependency(const QStringList &names);
    QString extractMethod(const QString &line);
    QString decodeType(QString param);
    QString decodeParams(QString line);
    QString comment();

    // Read methods
    bool readLine(const QString &line);
    void readNotification(const QString &line);
    void readRequest(const QString &line);
    void readCode(QString line);
    void readComment(const QString &line);
    void readEnum(const QString &line);
    void readType(const QString &line);
    void readInterface(const QString &line);
    void readInterfaceProperties(const QString &line);

    QString decodeNotificationParams(const QString &line);
    QString decodeRequestResults(const QString &line);
    QString decodeTypeDataType(const QString &params);
    QString decodePropertyDataType(const QString &line);
    bool specialProperty(const QString &line);

private:
    std::vector<State> m_states = {None};

    Data m_data;

    QString m_comment;
    Data::Notification m_notification;
    Data::Request m_request;
    Data::Enumeration m_enumeration;
    Data::Type m_type;
    std::vector<Data::Interface> m_interfaces;
};

#endif // SPECPARSER_H
