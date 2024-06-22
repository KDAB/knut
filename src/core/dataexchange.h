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

struct DataValidationEntry
{
    Q_GADGET

    Q_PROPERTY(QString function MEMBER function)
    Q_PROPERTY(QString member MEMBER member)
    Q_PROPERTY(QStringList arguments MEMBER arguments)

public:
    Q_INVOKABLE QString toString() const;

    QString function;
    QString member;
    QStringList arguments;

    bool operator==(const DataValidationEntry &other) const = default;
};

struct DataExchange
{
    Q_GADGET

    Q_PROPERTY(QString className MEMBER className)
    Q_PROPERTY(QVector<Core::DataExchangeEntry> entries MEMBER entries FINAL)
    Q_PROPERTY(QVector<Core::DataValidationEntry> validators MEMBER validators FINAL)
    Q_PROPERTY(Core::RangeMark range MEMBER range)
    Q_PROPERTY(bool isValid READ isValid FINAL)

private:
    friend class CppDocument;
    DataExchange(QString _className, const QueryMatch &ddxFunction);

public:
    DataExchange() = default;

    bool isValid() const;

    Q_INVOKABLE QString toString() const;

    QString className;
    QVector<DataExchangeEntry> entries;
    QVector<DataValidationEntry> validators;
    RangeMark range;
};

} // namespace Core
