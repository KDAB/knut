#include "lexer.h"

#include <QDir>
#include <QHash>

#include <spdlog/spdlog.h>

namespace RcCore {

static QHash<QString, Keywords> KeywordMap = {
    // Used in MENUITEM - not handled in Qt
    {"ACCELERATORS", Keywords::ACCELERATORS},
    {"AFX_DIALOG_LAYOUT", Keywords::AFX_DIALOG_LAYOUT},
    {"BITMAP", Keywords::BITMAP},
    {"CURSOR", Keywords::CURSOR},
    {"DESIGNINFO", Keywords::DESIGNINFO},
    {"DIALOG", Keywords::DIALOG},
    {"DIALOGEX", Keywords::DIALOGEX},
    {"DLGINIT", Keywords::DLGINIT},
    {"FONT", Keywords::FONT},
    {"HTML", Keywords::HTML},
    {"ICON", Keywords::ICON},
    {"MENU", Keywords::MENU},
    {"MENUEX", Keywords::MENUEX},
    {"MESSAGETABLE", Keywords::MESSAGETABLE},
    {"PNG", Keywords::PNG},
    {"POPUP", Keywords::POPUP},
    {"RCDATA", Keywords::RCDATA},
    {"REGISTRY", Keywords::REGISTRY},
    {"STRINGTABLE", Keywords::STRINGTABLE},
    {"TEXTINCLUDE", Keywords::TEXTINCLUDE},
    {"TOOLBAR", Keywords::TOOLBAR},
    {"VERSIONINFO", Keywords::VERSIONINFO},
    {"RT_RIBBON_XML", Keywords::RT_RIBBON_XML},
    {"PRELOAD", Keywords::IGNORE_16BITS},
    {"LOADONCALL", Keywords::IGNORE_16BITS},
    {"FIXED", Keywords::IGNORE_16BITS},
    {"MOVEABLE", Keywords::IGNORE_16BITS},
    {"DISCARDABLE", Keywords::IGNORE_16BITS},
    {"PURE", Keywords::IGNORE_16BITS},
    {"IMPURE", Keywords::IGNORE_16BITS},
    {"SHARED", Keywords::IGNORE_16BITS},
    {"NONSHARED", Keywords::IGNORE_16BITS},
    {"BEGIN", Keywords::BEGIN},
    {"END", Keywords::END},
    {"SEPARATOR", Keywords::SEPARATOR},
    {"MFT_SEPARATOR", Keywords::SEPARATOR},
    {"BUTTON", Keywords::BUTTON},
    {"NOT", Keywords::NOT},
    {"CHECKED", Keywords::CHECKED},
    {"MFS_CHECKED", Keywords::CHECKED},
    {"GRAYED", Keywords::GRAYED},
    {"MFS_GRAYED", Keywords::GRAYED},
    {"MFS_DISABLED", Keywords::INACTIVE},
    {"HELP", Keywords::HELP},
    {"INACTIVE", Keywords::INACTIVE},
    {"MENUBARBREAK", Keywords::MENUBARBREAK},
    {"MFT_MENUBARBREAK", Keywords::MENUBARBREAK},
    {"MENUBREAK", Keywords::MENUBREAK},
    {"MFT_MENUBREAK", Keywords::MENUBREAK},
    {"MFT_STRING", Keywords::MFTSTRING},
    {"MFS_ENABLED", Keywords::MFSENABLED},
    {"MFT_RIGHTJUSTIFY", Keywords::MFTRIGHTJUSTIFY},
    {"ALT", Keywords::ALT},
    {"ASCII", Keywords::ASCII},
    {"NOINVERT", Keywords::NOINVERT},
    {"SHIFT", Keywords::SHIFT},
    {"VIRTKEY", Keywords::VIRTKEY},
    {"CAPTION", Keywords::CAPTION},
    {"CHARACTERISTICS", Keywords::CHARACTERISTICS},
    {"CLASS", Keywords::CLASS},
    {"EXSTYLE", Keywords::EXSTYLE},
    {"LANGUAGE", Keywords::LANGUAGE},
    {"MENUITEM", Keywords::MENUITEM},
    {"STYLE", Keywords::STYLE},
    {"VERSION", Keywords::VERSION},
    {"AUTO3STATE", Keywords::AUTO3STATE},
    {"AUTOCHECKBOX", Keywords::AUTOCHECKBOX},
    {"AUTORADIOBUTTON", Keywords::AUTORADIOBUTTON},
    {"CHECKBOX", Keywords::CHECKBOX},
    {"COMBOBOX", Keywords::COMBOBOX},
    {"CONTROL", Keywords::CONTROL},
    {"CTEXT", Keywords::CTEXT},
    {"DEFPUSHBUTTON", Keywords::DEFPUSHBUTTON},
    {"EDITTEXT", Keywords::EDITTEXT},
    {"GROUPBOX", Keywords::GROUPBOX},
    {"LISTBOX", Keywords::LISTBOX},
    {"LTEXT", Keywords::LTEXT},
    {"PUSHBOX", Keywords::PUSHBOX},
    {"PUSHBUTTON", Keywords::PUSHBUTTON},
    {"RADIOBUTTON", Keywords::RADIOBUTTON},
    {"RTEXT", Keywords::RTEXT},
    {"SCROLLBAR", Keywords::SCROLLBAR},
    {"STATE3", Keywords::STATE3},
};

//=============================================================================
// Parser::Token
//=============================================================================

QString Token::toString() const
{
    if (data.index() == 1)
        return std::get<QString>(data);
    return KeywordMap.key(std::get<Keywords>(data));
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
        return KeywordMap.key(toKeyword());
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
    return KeywordMap.keys();
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

    spdlog::critical("{}({}): general lexer error reading next token", m_fileName.toStdString(), m_stream.line());
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
    if (auto it = KeywordMap.find(word); it != KeywordMap.end())
        return {Token::Keyword, it.value()};
    return {Token::Word, word};
}

} // namespace RcCore
