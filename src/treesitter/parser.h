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

private:
    TSParser *m_parser;
};

} // namespace treesitter
