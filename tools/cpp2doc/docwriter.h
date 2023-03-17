#pragma once

#include "data.h"

#include <QHash>

class DocWriter
{
public:
    DocWriter(Data data);

    void saveDocumentation();

private:
    void writeToc();
    void writeTypeFile(const Data::TypeBlock &type);
    std::vector<Data::PropertyBlock> propertyForType(const Data::TypeBlock &type) const;
    std::vector<Data::MethodBlock> methodForType(const Data::TypeBlock &type) const;
    std::vector<Data::SignalBlock> signalForType(const Data::TypeBlock &type) const;
    QString methodToString(const Data::QmlMethod &method, bool summary) const;
    QString typeToString(QString type) const;

private:
    Data m_data;
    QHash<QString, QString> m_typeFileMap;
    QHash<QString, QStringList> m_navMap;
};
