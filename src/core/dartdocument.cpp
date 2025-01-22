/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "dartdocument.h"

#include "codedocument_p.h"
#include "symbol.h"

namespace {

QList<Core::Symbol *> queryAllSymbols(Core::CodeDocument *const document)
{
    Q_UNUSED(document);
    // TODO
    spdlog::warn("DartDocument::symbols: Symbols are not (yet) supported for C# code. "
                 "Some functionality may not work as expected!");
    return {};
}

} // anonymous namespace

namespace Core {

DartDocument::DartDocument(QObject *parent)
    : CodeDocument(Type::Dart, parent)
{
    helper()->querySymbols = queryAllSymbols;
}

DartDocument::~DartDocument() = default;

} // namespace Core
