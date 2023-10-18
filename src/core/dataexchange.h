#pragma once

#include "rangemark.h"

#include <QObject>

namespace Core {

class QueryMatch;

struct DataExchangeEntry
{
    Q_GADGET

    Q_PROPERTY(QString function MEMBER function)
    Q_PROPERTY(QString idc MEMBER idc)
    Q_PROPERTY(QString member MEMBER member)
public:
    Q_INVOKABLE QString toString() const;

    QString function;
    QString idc;
    QString member;

    bool operator==(const DataExchangeEntry &other) const = default;
};

struct DataExchange
{
    Q_GADGET

    Q_PROPERTY(QString className MEMBER className)
    Q_PROPERTY(QVector<Core::DataExchangeEntry> entries MEMBER entries FINAL)
    Q_PROPERTY(Core::RangeMark range MEMBER range)
    Q_PROPERTY(bool isValid READ isValid FINAL)

private:
    friend class CppDocument;
    DataExchange(const QString &_className, const QueryMatch &query);

public:
    DataExchange() = default;

    bool isValid() const;

    Q_INVOKABLE QString toString() const;

    QString className;
    QVector<DataExchangeEntry> entries;
    RangeMark range;
};

} // namespace Core
