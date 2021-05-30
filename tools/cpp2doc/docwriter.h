#pragma once

#include "data.h"

#include <QHash>

class DocWriter
{
public:
    DocWriter(Data data);

    void saveDocumentation();

private:
    void writeTypeFile(const Data::TypeBlock &type);
    std::vector<Data::PropertyBlock> propertyForType(const QString &typeName) const;
    std::vector<Data::MethodBlock> methodForType(const QString &typeName) const;
    QString methodToString(const Data::QmlMethod &method, bool addLink) const;

private:
    Data m_data;
    QHash<QString, QString> m_typeFileMap;
    QHash<QString, QStringList> m_navMap;
};
