#ifndef SPECPARSER_H
#define SPECPARSER_H

#include "data.h"

#include <vector>

#include <nlohmann/json.hpp>

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

class MetaModelSpecParser
{
public:
    MetaData parse(const QString &fileName);

private:
    void readSpecialInterface(const MetaData::InterfacePtr &interface);

    MetaData::TypePtr readType(const nlohmann::json &object);
    QString readComment(const nlohmann::json &object);
    MetaData::TypePtr readProperty(const nlohmann::json &object);

    void parseRequest(const nlohmann::json &object);
    void parseNotification(const nlohmann::json &object);
    void parseStructure(const nlohmann::json &object);
    void parseEnumeration(const nlohmann::json &object);
    void parseTypeAlias(const nlohmann::json &object);
    bool parseLiteral(const nlohmann::json &object, const QString &name);

    void parseRequests(const nlohmann::json &json);
    void parseNotifications(const nlohmann::json &json);
    void parseStructures(const nlohmann::json &json);
    void parseEnumerations(const nlohmann::json &json);
    void parseTypeAliases(const nlohmann::json &json);

    void handleMixins();

private:
    MetaData m_data;
    QList<MetaData::TypePtr> m_typeStack;

    QStringList m_path;
};

#endif // SPECPARSER_H
