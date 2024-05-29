/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QString>

struct TSParser;
struct TSLanguage;

namespace treesitter {

class Tree;

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

    std::optional<Tree> parseString(const QString &text, const Tree *old_tree = nullptr);

    const TSLanguage *language() const;

private:
    TSParser *m_parser;
};

} // namespace treesitter
