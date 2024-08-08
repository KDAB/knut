/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qmldocument.h"
#include "codedocument_p.h"
#include <kdalgorithms.h>

namespace {
using namespace Core;

auto queryUIObjectSymbols(CodeDocument *const document) -> QList<Core::Symbol *>
{
    const auto queryString = QString(R"EOF(
        (ui_object_definition type_name : (_) @name @selectionRange) @range
    )EOF");
    auto uiObjectDefinitions = document->query(queryString);
    auto objectsToSymbol = [document](const QueryMatch &match) {
        return Symbol::makeSymbol(document, match, Symbol::Kind::Object);
    };
    return kdalgorithms::transformed<QList<Symbol *>>(uiObjectDefinitions, objectsToSymbol);
}
auto queryFunctionSymbols(CodeDocument *const document) -> QList<Core::Symbol *>
{
    const auto queryString = QString(R"EOF(
        (function_declaration
        name:(_) @name @selectionRange
        parameters: (formal_parameters
            [(required_parameter) @parameter _]*
        ) @parameters
        body: (_) @body
        ) @range
    )EOF");
    auto functions = document->query(queryString);

    auto function_to_symbol = [document](const QueryMatch &match) {
        return Symbol::makeSymbol(document, match, Symbol::Kind::Function);
    };

    return kdalgorithms::transformed<QList<Symbol *>>(functions, function_to_symbol);
}

auto queryPropertySymbols(CodeDocument *const document) -> QList<Core::Symbol *>
{
    const auto queryString = QString(R"EOF(
        (ui_binding
        name: (_) @name @selectionRange
        value: ((_)@type)
        @value) @range

    )EOF");
    auto members = document->query(queryString);

    auto member_to_symbol = [document](const QueryMatch &match) {
        return Symbol::makeSymbol(document, match, Symbol::Kind::Field);
    };

    return kdalgorithms::transformed<QList<Symbol *>>(members, member_to_symbol);
}

auto queryAllSymbols(CodeDocument *const document) -> QList<Core::Symbol *>
{
    auto symbols = ::queryUIObjectSymbols(document);
    symbols.append(::queryFunctionSymbols(document));
    symbols.append(::queryPropertySymbols(document));

    return symbols;
}

}

namespace Core {

QmlDocument::QmlDocument(QObject *parent)
    : CodeDocument(Type::Qml, parent)
{
    helper()->querySymbols = ::queryAllSymbols;
}

QmlDocument::~QmlDocument() = default;
}
