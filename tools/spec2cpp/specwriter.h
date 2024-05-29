#pragma once

#include "data.h"

class QTextStream;

class SpecWriter
{
public:
    SpecWriter(Data data);

    void saveNotifications();
    void saveRequests();
    void saveCode();

private:
    void cleanCode();

    QString writeEnums();
    QString writeJsonEnums();
    QString writeTypesAndInterfaces();
    QString writeType(const Data::Type &type);
    QString writeMainInterface(const Data::Interface &interface);
    QString writeJsonInterface(const Data::Interface &interface, QStringList parent = {});

private:
    Data m_data;
};

class MetaSpecWriter
{
public:
    MetaSpecWriter(MetaData data);

    void saveNotifications();
    void saveRequests();
    void saveCode();

private:
    void cleanCode();

    QString writeEnums();
    QString writeJsonEnums();
    QString writeTypesAndInterfaces();
    QString writeType(const MetaData::TypePtr &type);
    QString writeProperty(const MetaData::TypePtr &property, const QString &interface);
    QString writeChildInterface(const MetaData::TypePtr &type, QStringList parent);
    QString writeMainInterface(const MetaData::InterfacePtr &interface);
    QString writeJsonInterface(const MetaData::InterfacePtr &interface, QStringList parent = {});

private:
    MetaData m_data;
    std::vector<MetaData::TypePtr> m_rootTypes;
};
