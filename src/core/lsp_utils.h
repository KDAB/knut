/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "lsp/types.h"
#include "rangemark.h"
#include "textrange.h"

#include <QString>
#include <string>

namespace Core {
class TextDocument;
}

namespace Core::Utils {

Lsp::Position lspFromPos(const TextDocument &textDocument, int pos);

int lspToPos(const TextDocument &textDocument, const Lsp::Position &pos);

TextRange lspToRange(const TextDocument &textDocument, const Lsp::Range &range);

RangeMarkList lspToRangeMarkList(const std::vector<Lsp::Location> &locations);

QString removeTypeAliasInformation(const QString &typeInfo);

} // namespace Core::Utils
