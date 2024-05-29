#include "parser.h"
#include "tree.h"

#include <tree_sitter/api.h>
#include <utility>

namespace treesitter {

Parser::Parser(TSLanguage *language)
    : m_parser(ts_parser_new())
{
    ts_parser_set_language(m_parser, language);
}

Parser::Parser(Parser &&other) noexcept
    : m_parser(other.m_parser)
{
    other.m_parser = nullptr;
}

Parser::~Parser()
{
    if (m_parser) {
        ts_parser_delete(m_parser);
    }
}

Parser &Parser::operator=(Parser &&other) noexcept
{
    Parser(std::move(other)).swap(*this);
    return *this;
}

void Parser::swap(Parser &other) noexcept
{
    std::swap(m_parser, other.m_parser);
}

std::optional<Tree> Parser::parseString(const QString &text, const Tree *old_tree)
{
    auto tree =
        ts_parser_parse_string_encoding(m_parser, old_tree ? old_tree->m_tree : nullptr, (const char *)text.constData(),
                                        static_cast<uint32_t>(text.size() * sizeof(QChar)), TSInputEncodingUTF16);

    // TreeSitter may return a nullptr. See: https://tree-sitter.docsforge.com/master/api/ts_parser_parse/
    // In this case, return an empty optional.
    return tree ? Tree(tree) : std::optional<Tree> {};
}

const TSLanguage *Parser::language() const
{
    return ts_parser_language(m_parser);
}

}
