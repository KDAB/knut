/*
 This file is part of Knut.

 SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

 SPDX-License-Identifier: GPL-3.0-only

 Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "symbol.h"

namespace Core {

class TypedSymbol : public Symbol
{
    Q_OBJECT

    Q_PROPERTY(QString type READ type CONSTANT)

public:
    QString type() const;

private:
    friend class Symbol;
    TypedSymbol(QObject *parent, const QueryMatch &match, Kind kind);

    QString m_type;
};

}
