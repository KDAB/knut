/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "rangemark.h"

#include <QObject>

namespace Core {

struct MessageMapEntry
{
    Q_GADGET

    Q_PROPERTY(Core::RangeMark range MEMBER range FINAL)
    Q_PROPERTY(QList<Core::RangeMark> parameters MEMBER parameters FINAL)
    Q_PROPERTY(QString name MEMBER name FINAL)
    Q_PROPERTY(bool isValid READ isValid FINAL)
public:
    Q_INVOKABLE QString toString() const;

    bool isValid() const;

    RangeMark range;
    QString name;
    QList<RangeMark> parameters;

    bool operator==(const MessageMapEntry &other) const = default;
};

using MessageMapEntryList = QList<Core::MessageMapEntry>;

struct MessageMap
{
    Q_GADGET

    Q_PROPERTY(QList<Core::MessageMapEntry> entries MEMBER entries)
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
    Q_INVOKABLE Core::MessageMapEntryList getAll(const QString &name) const;

    Q_INVOKABLE QString toString() const;

    QString className;
    QString superClass;
    QList<MessageMapEntry> entries;
    RangeMark range;
};

} // namespace core

Q_DECLARE_METATYPE(Core::MessageMap)
Q_DECLARE_METATYPE(Core::MessageMapEntry)
