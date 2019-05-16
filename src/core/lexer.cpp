#include "lexer.h"

#include "logging.h"

#include <QHash>

namespace Parser {

static QHash<QString, Keywords> KeywordMap = {
    {QStringLiteral("ACCELERATORS"), Keywords::ACCELERATORS},
    {QStringLiteral("AFX_DIALOG_LAYOUT"), Keywords::AFX_DIALOG_LAYOUT},
    {QStringLiteral("BITMAP"), Keywords::BITMAP},
    {QStringLiteral("CURSOR"), Keywords::CURSOR},
    {QStringLiteral("DESIGNINFO"), Keywords::DESIGNINFO},
    {QStringLiteral("DIALOG"), Keywords::DIALOG},
    {QStringLiteral("DIALOGEX"), Keywords::DIALOGEX},
    {QStringLiteral("DLGINIT"), Keywords::DLGINIT},
    {QStringLiteral("FONT"), Keywords::FONT},
    {QStringLiteral("HTML"), Keywords::HTML},
    {QStringLiteral("ICON"), Keywords::ICON},
    {QStringLiteral("MENU"), Keywords::MENU},
    {QStringLiteral("MENUEX"), Keywords::MENUEX},
    {QStringLiteral("MESSAGETABLE"), Keywords::MESSAGETABLE},
    {QStringLiteral("PNG"), Keywords::PNG},
    {QStringLiteral("POPUP"), Keywords::POPUP},
    {QStringLiteral("RCDATA"), Keywords::RCDATA},
    {QStringLiteral("STRINGTABLE"), Keywords::STRINGTABLE},
    {QStringLiteral("TEXTINCLUDE"), Keywords::TEXTINCLUDE},
    {QStringLiteral("TOOLBAR"), Keywords::TOOLBAR},
    {QStringLiteral("VERSIONINFO"), Keywords::VERSIONINFO},
    {QStringLiteral("BEGIN"), Keywords::BEGIN},
    {QStringLiteral("END"), Keywords::END},
    {QStringLiteral("SEPARATOR"), Keywords::SEPARATOR},
    {QStringLiteral("BUTTON"), Keywords::BUTTON},
    {QStringLiteral("NOT"), Keywords::NOT},
    {QStringLiteral("CHECKED"), Keywords::CHECKED},
    {QStringLiteral("GRAYED"), Keywords::GRAYED},
    {QStringLiteral("HELP"), Keywords::HELP},
    {QStringLiteral("INACTIVE"), Keywords::INACTIVE},
    {QStringLiteral("MENUBARBREAK"), Keywords::MENUBARBREAK},
    {QStringLiteral("MENUBREAK"), Keywords::MENUBREAK},
    {QStringLiteral("ALT"), Keywords::ALT},
    {QStringLiteral("ASCII"), Keywords::ASCII},
    {QStringLiteral("NOINVERT"), Keywords::NOINVERT},
    {QStringLiteral("SHIFT"), Keywords::SHIFT},
    {QStringLiteral("VIRTKEY"), Keywords::VIRTKEY},
    {QStringLiteral("CAPTION"), Keywords::CAPTION},
    {QStringLiteral("CHARACTERISTICS"), Keywords::CHARACTERISTICS},
    {QStringLiteral("CLASS"), Keywords::CLASS},
    {QStringLiteral("EXSTYLE"), Keywords::EXSTYLE},
    {QStringLiteral("LANGUAGE"), Keywords::LANGUAGE},
    {QStringLiteral("MENUITEM"), Keywords::MENUITEM},
    {QStringLiteral("STYLE"), Keywords::STYLE},
    {QStringLiteral("VERSION"), Keywords::VERSION},
    {QStringLiteral("AUTO3STATE"), Keywords::AUTO3STATE},
    {QStringLiteral("AUTOCHECKBOX"), Keywords::AUTOCHECKBOX},
    {QStringLiteral("AUTORADIOBUTTON"), Keywords::AUTORADIOBUTTON},
    {QStringLiteral("CHECKBOX"), Keywords::CHECKBOX},
    {QStringLiteral("COMBOBOX"), Keywords::COMBOBOX},
    {QStringLiteral("CONTROL"), Keywords::CONTROL},
    {QStringLiteral("CTEXT"), Keywords::CTEXT},
    {QStringLiteral("DEFPUSHBUTTON"), Keywords::DEFPUSHBUTTON},
    {QStringLiteral("EDITTEXT"), Keywords::EDITTEXT},
    {QStringLiteral("GROUPBOX"), Keywords::GROUPBOX},
    {QStringLiteral("LISTBOX"), Keywords::LISTBOX},
    {QStringLiteral("LTEXT"), Keywords::LTEXT},
    {QStringLiteral("PUSHBOX"), Keywords::PUSHBOX},
    {QStringLiteral("PUSHBUTTON"), Keywords::PUSHBUTTON},
    {QStringLiteral("RADIOBUTTON"), Keywords::RADIOBUTTON},
    {QStringLiteral("RTEXT"), Keywords::RTEXT},
    {QStringLiteral("SCROLLBAR"), Keywords::SCROLLBAR},
    {QStringLiteral("STATE3"), Keywords::STATE3},
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
        return QStringLiteral(",");
    case Token::Operator_Or:
        return QStringLiteral("|");
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
    skipWhile([](const auto &c) { return c.isSpace(); });
    m_current.reset();
}

void Lexer::skipLine()
{
    skipWhile([](const auto &c) { return c != QLatin1Char('\n'); });
    m_current.reset();
}

void Lexer::skipComma()
{
    skipSpace();
    if (m_stream.peek() == QLatin1Char(','))
        m_stream.next();
    m_current.reset();
}

void Lexer::skipScope()
{
    int scope = 0;
    bool inScope = false;
    while (!m_stream.atEnd() && (scope != 0 || !inScope)) {
        skipSpace();
        const QChar &ch = m_stream.peek();
        if (ch == QLatin1Char('B') || ch == QLatin1Char('E')) {
            const QString word = readWhile([](const auto &c) { return c.isLetter(); });
            if (word == QLatin1String("BEGIN")) {
                ++scope;
                inScope = true;
            } else if (word == QLatin1String("END"))
                --scope;
        }
        skipLine();
    }
    m_current.reset();
}

void Lexer::skipToBegin()
{
    while (!m_stream.atEnd()) {
        skipSpace();
        const QChar &ch = m_stream.peek();
        if (ch == QLatin1Char('B')) {
            const QString word = readWhile([](const auto &c) { return c.isLetter(); });
            if (word == QLatin1String("BEGIN"))
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
    if (ch == QLatin1Char('/')) { // Skip comments
        skipLine();
        return readNext();
    }
    if (ch == QLatin1Char('"'))
        return readString();
    if (ch == QLatin1Char(',')) {
        m_stream.next();
        return Token {Token::Operator_Comma, {}};
    }
    if (ch == QLatin1Char('|')) {
        m_stream.next();
        return Token {Token::Operator_Or, {}};
    }
    if (ch == QLatin1Char('#'))
        return readDirective();
    if (ch.isNumber())
        return readNumber();
    if (ch == QLatin1Char('-'))
        return readNumber();
    if (ch.isLetter())
        return readWord();

    qCCritical(TOKENIZER) << "Lexer error line:" << m_stream.line();
    return {};
}

Token Lexer::readDirective()
{
    m_stream.next(); // Read the first '#'
    return {Token::Directive, readWhile([](const auto &c) { return c.isLetter(); })};
}

Token Lexer::readString()
{
    QString str;
    m_stream.next(); // Read the first '"'
    bool escaped = false;
    while (!m_stream.atEnd()) {
        const QChar &ch = m_stream.next();
        if (escaped) {
            if (ch == QLatin1Char('n'))
                str += QLatin1Char('\n');
            else if (ch == QLatin1Char('t'))
                str += QLatin1Char('\t');
            else if (ch == QLatin1Char('"'))
                str += QLatin1Char('"');
            else
                str += QLatin1Char('\\') + ch;
            escaped = false;
        } else if (ch == QLatin1Char('\\')) {
            escaped = true;
        } else if (ch == QLatin1Char('"')) {
            // " are escaped with "" in RC files
            if (m_stream.peek() == QLatin1Char('"'))
                escaped = true;
            else
                break;
        } else {
            str += ch;
        }
    }
    return {Token::String, str};
}

Token Lexer::readNumber()
{
    QString text = m_stream.next();
    if (text[0] == QLatin1Char('0') && m_stream.peek() == QLatin1Char('x')) {
        text += m_stream.next();
        text += readWhile([](const auto &c) { return c.isLetterOrNumber(); });
        return {Token::Word, text};
    }

    text += readWhile([](const auto &c) { return c.isNumber(); });
    return {Token::Integer, text.toInt()};
}

Token Lexer::readWord()
{
    const QString word = readWhile([](const auto &c) { return c.isLetterOrNumber() || c == QLatin1Char('_'); });
    if (auto it = KeywordMap.find(word); it != KeywordMap.end())
        return {Token::Keyword, it.value()};
    return {Token::Word, word};
}

}
