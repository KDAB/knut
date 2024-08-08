/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/document.h"
#include <QString>
#include <tree_sitter/api.h>
#include <vector>

struct TSParser;
struct TSLanguage;

namespace treesitter {

class Tree;
using Range = TSRange;

class Parser
{
public:
    Parser(TSLanguage *language);

    Parser(const Parser &) = delete;
    Parser(Parser &&) noexcept;

    Parser &operator=(const Parser &) = delete;
    Parser &operator=(Parser &&) noexcept;

    ~Parser();

    void swap(Parser &other) noexcept;

    std::optional<Tree> parseString(const QString &text, const Tree *old_tree = nullptr) const;

    /**
     * Parse only the given ranges.
     * Note: if the ranges are empty, the entire document is parsed.
     *
     * From tree_sitter/api.h:
     * [The] given ranges must be ordered from earliest to latest in the document,
     * and they must not overlap. That is, the following must hold for all
     * `i` < `length - 1`: ranges[i].end_byte <= ranges[i + 1].start_byte
     *
     * If this requirement is not satisfied, the operation will fail, the ranges
     * will not be assigned, and this function will return `false`. On success,
     * this function returns `true`
     */
    bool setIncludedRanges(const QList<Range> &ranges);

    const TSLanguage *language() const;

    static TSLanguage *getLanguage(Core::Document::Type type);

private:
    TSParser *m_parser;
};

} // namespace treesitter
