/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "symbol.h"

#include <QList>
#include <QString>

namespace Core {

class ClassSymbol : public Symbol
{
    Q_OBJECT

    Q_PROPERTY(QList<Symbol *> members READ members CONSTANT)

    QList<Symbol *> findMembers() const;

protected:
    friend class Symbol;
    ClassSymbol(QObject *parent, const QueryMatch &match, Kind kind);

    // mutable for lazy initialization
    mutable std::optional<QList<Symbol *>> m_members;

public:
    const QList<Symbol *> &members() const;
    QString description() const override;
};
bool operator==(const ClassSymbol &left, const ClassSymbol &right);

} // namespace Core

Q_DECLARE_METATYPE(Core::ClassSymbol)
