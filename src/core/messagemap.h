#pragma once

#include "rangemark.h"
#include <QObject>

namespace Core {

struct MessageMapEntry
{
    Q_GADGET

    Q_PROPERTY(Core::RangeMark range MEMBER range FINAL)
    Q_PROPERTY(QVector<Core::RangeMark> parameters MEMBER parameters FINAL)
    Q_PROPERTY(QString name MEMBER name FINAL)
    Q_PROPERTY(bool isValid READ isValid FINAL)
public:
    Q_INVOKABLE QString toString() const;

    bool isValid() const;

    RangeMark range;
    QString name;
    QVector<RangeMark> parameters;

    bool operator==(const MessageMapEntry &other) const = default;
};

struct MessageMap
{
    Q_GADGET

    Q_PROPERTY(QVector<Core::MessageMapEntry> entries MEMBER entries)
    Q_PROPERTY(Core::RangeMark range MEMBER range)
    Q_PROPERTY(QString className MEMBER className)
    Q_PROPERTY(QString superClass MEMBER superClass)
    Q_PROPERTY(bool isValid READ isValid FINAL)

private:
    friend class CppDocument;
    MessageMap(const class QueryMatch &match);

public:
    MessageMap() = default;

    bool isValid() const;

    Q_INVOKABLE Core::MessageMapEntry get(const QString &name) const;
    Q_INVOKABLE QVector<Core::MessageMapEntry> getAll(const QString &name) const;

    Q_INVOKABLE QString toString() const;

    QString className;
    QString superClass;
    QVector<MessageMapEntry> entries;
    RangeMark range;
};

} // namespace core

Q_DECLARE_METATYPE(Core::MessageMap)
Q_DECLARE_METATYPE(Core::MessageMapEntry)
