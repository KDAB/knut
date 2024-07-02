/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "querymatch.h"
#include "textlocation.h"
#include "textrange.h"

#include <QList>

namespace Core {

class CodeDocument;
class ClassSymbol;
class FunctionSymbol;

class Symbol : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(Kind kind READ kind CONSTANT)
    Q_PROPERTY(Core::TextRange range READ range CONSTANT)
    Q_PROPERTY(Core::TextRange selectionRange READ selectionRange CONSTANT)

public:
    // Follow the LSP SymbolKind enum
    enum Kind {
        File = 1,
        Module = 2,
        Namespace = 3,
        Package = 4,
        Class = 5,
        Method = 6,
        Property = 7,
        Field = 8,
        Constructor = 9,
        Enum = 10,
        Interface = 11,
        Function = 12,
        Variable = 13,
        Constant = 14,
        String = 15,
        Number = 16,
        Boolean = 17,
        Array = 18,
        Object = 19,
        Key = 20,
        Null = 21,
        EnumMember = 22,
        Struct = 23,
        Event = 24,
        Operator = 25,
        TypeParameter = 26,
    };
    Q_ENUM(Kind)

protected:
    Symbol(QObject *parent, const QueryMatch &match, Kind kind);

    QString m_name;
    Kind m_kind;
    TextRange m_range;
    TextRange m_selectionRange;
    QueryMatch m_queryMatch;

    CodeDocument *document() const;

public:
    static Symbol *makeSymbol(QObject *parent, const QueryMatch &match, Kind kind);

    Q_INVOKABLE bool isClass() const;
    Core::ClassSymbol *toClass();

    Q_INVOKABLE bool isFunction() const;
    Core::FunctionSymbol *toFunction();

    QString name() const;
    virtual QString description() const;
    Kind kind() const;
    Core::TextRange range() const;
    Core::TextRange selectionRange() const;

    // As per KNUT-163, these are no longer public API.
    // They are only used internally by the editor/GUI and not available from QML/JS.
    // As this relies on the clangd LSP, it is not reliable enough to use for scripting.
    QList<Core::TextLocation> references() const;

    Q_INVOKABLE void select();

    bool operator==(const Symbol &) const;

private:
    void assignContext(const QList<Symbol *> &contexts);

    friend class CodeDocument;
    friend class TreeSitterHelper;
};

using SymbolList = QList<Core::Symbol *>;

} // namespace Core

Q_DECLARE_METATYPE(Core::Symbol)
Q_DECLARE_METATYPE(QList<Core::Symbol *>)
