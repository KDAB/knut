/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "document.h"
#include "symbol.h"

#include <QList>
#include <QString>

namespace Core {

struct FunctionArgument
{
    Q_GADGET

    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(QString name MEMBER name)

public:
    QString type;
    QString name;
};
bool operator==(const FunctionArgument &left, const FunctionArgument &right);

class FunctionSymbol : public Symbol
{
public:
    Q_OBJECT

    Q_PROPERTY(QString returnType READ returnType CONSTANT)
    Q_PROPERTY(QList<Core::FunctionArgument> arguments READ arguments CONSTANT)

protected:
    // necessary so the constructor can be accessed from the Symbol class.
    friend class Symbol;

    FunctionSymbol(QObject *parent, const QueryMatch &match, Kind kind);

    mutable std::optional<QString> m_returnType;
    mutable std::optional<QList<FunctionArgument>> m_arguments;

    QString returnTypeFromQueryMatch() const;
    QList<FunctionArgument> argumentsFromQueryMatch() const;

public:
    QString returnType() const;
    const QList<FunctionArgument> &arguments() const;
    QString signature() const;
    QString description() const override;
};
bool operator==(const FunctionSymbol &left, const FunctionSymbol &right);

} // namespace Core

Q_DECLARE_METATYPE(Core::FunctionArgument)
Q_DECLARE_METATYPE(Core::FunctionSymbol)
Q_DECLARE_METATYPE(QList<Core::FunctionArgument>)
