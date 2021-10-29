#ifndef SPECWRITER_H
#define SPECWRITER_H

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

#endif // SPECWRITER_H
