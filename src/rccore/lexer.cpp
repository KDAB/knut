/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "lexer.h"
#include "utils/log.h"

#include <QDir>
#include <QHash>

namespace RcCore {

#define COMMA ,
// clang-format off
Q_GLOBAL_STATIC_WITH_ARGS(QHash<QString COMMA Keywords>, KeywordMap, {
    // Used in MENUITEM - not handled in Qt
    {"ACCELERATORS" COMMA Keywords::ACCELERATORS} COMMA
    {"AFX_DIALOG_LAYOUT" COMMA Keywords::AFX_DIALOG_LAYOUT} COMMA
    {"BITMAP" COMMA Keywords::BITMAP} COMMA
    {"CURSOR" COMMA Keywords::CURSOR} COMMA
    {"DESIGNINFO" COMMA Keywords::DESIGNINFO} COMMA
    {"DIALOG" COMMA Keywords::DIALOG} COMMA
    {"DIALOGEX" COMMA Keywords::DIALOGEX} COMMA
    {"DLGINIT" COMMA Keywords::DLGINIT} COMMA
    {"FONT" COMMA Keywords::FONT} COMMA
    {"HTML" COMMA Keywords::HTML} COMMA
    {"ICON" COMMA Keywords::ICON} COMMA
    {"IMAGE" COMMA Keywords::IMAGE} COMMA
    {"MENU" COMMA Keywords::MENU} COMMA
    {"MENUEX" COMMA Keywords::MENUEX} COMMA
    {"MESSAGETABLE" COMMA Keywords::MESSAGETABLE} COMMA
    {"PNG" COMMA Keywords::PNG} COMMA
    {"POPUP" COMMA Keywords::POPUP} COMMA
    {"RCDATA" COMMA Keywords::RCDATA} COMMA
    {"REGISTRY" COMMA Keywords::REGISTRY} COMMA
    {"STRINGTABLE" COMMA Keywords::STRINGTABLE} COMMA
    {"TEXTINCLUDE" COMMA Keywords::TEXTINCLUDE} COMMA
    {"TOOLBAR" COMMA Keywords::TOOLBAR} COMMA
    {"VERSIONINFO" COMMA Keywords::VERSIONINFO} COMMA
    {"RT_RIBBON_XML" COMMA Keywords::RT_RIBBON_XML} COMMA
    {"PRELOAD" COMMA Keywords::IGNORE_16BITS} COMMA
    {"LOADONCALL" COMMA Keywords::IGNORE_16BITS} COMMA
    {"FIXED" COMMA Keywords::IGNORE_16BITS} COMMA
    {"MOVEABLE" COMMA Keywords::IGNORE_16BITS} COMMA
    {"DISCARDABLE" COMMA Keywords::IGNORE_16BITS} COMMA
    {"PURE" COMMA Keywords::IGNORE_16BITS} COMMA
    {"IMPURE" COMMA Keywords::IGNORE_16BITS} COMMA
    {"SHARED" COMMA Keywords::IGNORE_16BITS} COMMA
    {"NONSHARED" COMMA Keywords::IGNORE_16BITS} COMMA
    {"BEGIN" COMMA Keywords::BEGIN} COMMA
    {"END" COMMA Keywords::END} COMMA
    {"SEPARATOR" COMMA Keywords::SEPARATOR} COMMA
    {"MFT_SEPARATOR" COMMA Keywords::SEPARATOR} COMMA
    {"BUTTON" COMMA Keywords::BUTTON} COMMA
    {"NOT" COMMA Keywords::NOT} COMMA
    {"CHECKED" COMMA Keywords::CHECKED} COMMA
    {"MFS_CHECKED" COMMA Keywords::CHECKED} COMMA
    {"GRAYED" COMMA Keywords::GRAYED} COMMA
    {"MFS_GRAYED" COMMA Keywords::GRAYED} COMMA
    {"MFS_DISABLED" COMMA Keywords::INACTIVE} COMMA
    {"HELP" COMMA Keywords::HELP} COMMA
    {"INACTIVE" COMMA Keywords::INACTIVE} COMMA
    {"MENUBARBREAK" COMMA Keywords::MENUBARBREAK} COMMA
    {"MFT_MENUBARBREAK" COMMA Keywords::MENUBARBREAK} COMMA
    {"MENUBREAK" COMMA Keywords::MENUBREAK} COMMA
    {"MFT_MENUBREAK" COMMA Keywords::MENUBREAK} COMMA
    {"MFT_STRING" COMMA Keywords::MFTSTRING} COMMA
    {"MFS_ENABLED" COMMA Keywords::MFSENABLED} COMMA
    {"MFT_RIGHTJUSTIFY" COMMA Keywords::MFTRIGHTJUSTIFY} COMMA
    {"ALT" COMMA Keywords::ALT} COMMA
    {"ASCII" COMMA Keywords::ASCII} COMMA
    {"NOINVERT" COMMA Keywords::NOINVERT} COMMA
    {"SHIFT" COMMA Keywords::SHIFT} COMMA
    {"VIRTKEY" COMMA Keywords::VIRTKEY} COMMA
    {"CAPTION" COMMA Keywords::CAPTION} COMMA
    {"CHARACTERISTICS" COMMA Keywords::CHARACTERISTICS} COMMA
    {"CLASS" COMMA Keywords::CLASS} COMMA
    {"EXSTYLE" COMMA Keywords::EXSTYLE} COMMA
    {"LANGUAGE" COMMA Keywords::LANGUAGE} COMMA
    {"MENUITEM" COMMA Keywords::MENUITEM} COMMA
    {"STYLE" COMMA Keywords::STYLE} COMMA
    {"VERSION" COMMA Keywords::VERSION} COMMA
    {"AUTO3STATE" COMMA Keywords::AUTO3STATE} COMMA
    {"AUTOCHECKBOX" COMMA Keywords::AUTOCHECKBOX} COMMA
    {"AUTORADIOBUTTON" COMMA Keywords::AUTORADIOBUTTON} COMMA
    {"CHECKBOX" COMMA Keywords::CHECKBOX} COMMA
    {"COMBOBOX" COMMA Keywords::COMBOBOX} COMMA
    {"CONTROL" COMMA Keywords::CONTROL} COMMA
    {"CTEXT" COMMA Keywords::CTEXT} COMMA
    {"DEFPUSHBUTTON" COMMA Keywords::DEFPUSHBUTTON} COMMA
    {"EDITTEXT" COMMA Keywords::EDITTEXT} COMMA
    {"GROUPBOX" COMMA Keywords::GROUPBOX} COMMA
    {"LISTBOX" COMMA Keywords::LISTBOX} COMMA
    {"LTEXT" COMMA Keywords::LTEXT} COMMA
    {"PUSHBOX" COMMA Keywords::PUSHBOX} COMMA
    {"PUSHBUTTON" COMMA Keywords::PUSHBUTTON} COMMA
    {"RADIOBUTTON" COMMA Keywords::RADIOBUTTON} COMMA
    {"RTEXT" COMMA Keywords::RTEXT} COMMA
    {"SCROLLBAR" COMMA Keywords::SCROLLBAR} COMMA
    {"STATE3" COMMA Keywords::STATE3} COMMA
});
// clang-format on

//=============================================================================
// Parser::Token
//=============================================================================

QString Token::toString() const
{
    if (data.index() == 1)
        return std::get<QString>(data);
    return KeywordMap->key(std::get<Keywords>(data));
}

QString Token::prettyPrint() const
{
    switch (type) {
    case Token::Operator_Comma:
        return ",";
    case Token::Operator_Or:
        return "|";
    case Token::Directive:
        return QStringLiteral("#%1").arg(toString());
    case Token::String:
        return QStringLiteral("\"%1\"").arg(toString());
    case Token::Integer:
        return QString::number(toInt());
    case Token::Keyword:
        return KeywordMap->key(toKeyword());
    case Token::Word:
        return toString();
    }
    return {};
}

//=============================================================================
// Parser::Lexer
//=============================================================================
Lexer::Lexer(Stream input)
    : m_stream(std::move(input))
{
}

void Lexer::skipSpace()
{
    skipWhile([](const auto &c) {
        return c.isSpace();
    });
    m_current.reset();
}

void Lexer::skipLine()
{
    skipWhile([](const auto &c) {
        return c != '\n';
    });
    m_current.reset();
}

void Lexer::skipComma()
{
    skipSpace();
    if (m_stream.peek() == ',')
        m_stream.next();
    m_current.reset();
}

void Lexer::skipScope()
{
    skipToBegin();
    int scope = 1;
    while (!m_stream.atEnd() && (scope != 0)) {
        skipSpace();
        const QChar &ch = m_stream.peek();
        if (ch == 'B' || ch == 'E') {
            const QString word = readWhile([](const auto &c) {
                return c.isLetter();
            });
            if (word == "BEGIN")
                ++scope;
            else if (word == "END")
                --scope;
        }
        skipLine();
    }
    m_current.reset();
}

void Lexer::skipToBegin()
{
    if (m_current && m_current->type == Token::Keyword && m_current->toKeyword() == Keywords::BEGIN) {
        m_current.reset();
        return;
    }
    while (!m_stream.atEnd()) {
        skipSpace();
        const QChar &ch = m_stream.peek();
        if (ch == 'B') {
            const QString word = readWhile([](const auto &c) {
                return c.isLetter();
            });
            if (word == "BEGIN")
                return;
        }
        skipLine();
    }
    m_current.reset();
}

std::optional<Token> Lexer::next()
{
    auto token = m_current;
    m_current.reset();
    if (!token.has_value())
        token = readNext();
    return token;
}

std::optional<Token> Lexer::peek()
{
    if (!m_current.has_value())
        m_current = readNext();
    return m_current;
}

QList<QString> Lexer::keywords()
{
    return KeywordMap->keys();
}

std::optional<Token> Lexer::readNext()
{
    skipSpace();
    if (m_stream.atEnd())
        return {};

    const QChar &ch = m_stream.peek();
    if (ch == '/') { // Skip comments
        skipLine();
        return readNext();
    }
    if (ch == '"')
        return readString();
    if (ch == ',') {
        m_stream.next();
        return Token {Token::Operator_Comma, {}};
    }
    if (ch == '|') {
        m_stream.next();
        return Token {Token::Operator_Or, {}};
    }
    if (ch == '#')
        return readDirective();
    if (ch.isNumber())
        return readNumber();
    if (ch == '-')
        return readNumber();
    if (ch.isLetter())
        return readWord();
    if (ch == '<')
        return readInclude();

    CRITICAL("{}({}): general lexer error reading next token", m_fileName, m_stream.line());
    return {};
}

Token Lexer::readDirective()
{
    m_stream.next(); // Read the first '#'
    return {Token::Directive, readWhile([](const auto &c) {
                return c.isLetter();
            })};
}

Token Lexer::readString()
{
    QString str;
    m_stream.next(); // Read the first '"'
    bool escaped = false;
    while (!m_stream.atEnd()) {
        const QChar &ch = m_stream.next();
        if (escaped) {
            if (ch == 'n')
                str += '\n';
            else if (ch == 't')
                str += '\t';
            else if (ch == '"')
                str += '"';
            else
                str += '\\' + ch;
            escaped = false;
        } else if (ch == '\\') {
            escaped = true;
        } else if (ch == '"') {
            // " are escaped with "" in RC files
            if (m_stream.peek() == '"')
                escaped = true;
            else
                break;
        } else {
            str += ch;
        }
    }
    return {Token::String, str};
}

Token Lexer::readInclude()
{
    QString str;
    m_stream.next(); // Read the first '"'
    while (!m_stream.atEnd()) {
        const QChar &ch = m_stream.next();
        if (ch == '>')
            break;
        else
            str += ch;
    }
    return {Token::String, str};
}

Token Lexer::readNumber()
{
    QString text = m_stream.next();
    if (text[0] == '0' && m_stream.peek() == 'x') {
        text += m_stream.next();
        text += readWhile([](const auto &c) {
            return c.isLetterOrNumber();
        });
        return {Token::Word, text};
    }

    text += readWhile([](const auto &c) {
        return c.isNumber();
    });
    return {Token::Integer, text.toInt()};
}

Token Lexer::readWord()
{
    const QString word = readWhile([](const auto &c) {
        return c.isLetterOrNumber() || c == '_';
    });
    if (auto it = KeywordMap->find(word); it != KeywordMap->end())
        return {Token::Keyword, it.value()};
    return {Token::Word, word};
}

} // namespace RcCore
