#pragma once

#include "stream.h"

#include <QChar>
#include <QString>
#include <QVariant>

#include <optional>
#include <variant>

namespace RcCore {

// See the list and description here:
// https://docs.microsoft.com/en-us/windows/desktop/menurc/resource-definition-statements
enum class Keywords {
    // All keywords ignored
    // 16-bits only: PRELOAD, LOADONCALL, FIXED, MOVEABLE, DISCARDABLE, PURE, IMPURE, SHARED,
    // NONSHARED
    IGNORE_16BITS,

    // Resources
    ACCELERATORS,
    AFX_DIALOG_LAYOUT, // https://docs.microsoft.com/en-us/cpp/mfc/dynamic-layout?view=vs-Core
    BITMAP,
    CURSOR,
    DESIGNINFO,
    DIALOG,
    DIALOGEX,
    DLGINIT,
    FONT,
    HTML,
    ICON,
    MENU,
    MENUEX,
    MESSAGETABLE,
    PNG,
    POPUP,
    RCDATA,
    REGISTRY,
    STRINGTABLE,
    TEXTINCLUDE,
    TOOLBAR,
    VERSIONINFO,

    // Other
    BEGIN,
    END,
    SEPARATOR,
    BUTTON,
    NOT,

    // MenuItem
    CHECKED,
    GRAYED,
    HELP,
    INACTIVE,
    MENUBARBREAK,
    MENUBREAK,
    MFTSTRING,
    MFSENABLED,
    MFTRIGHTJUSTIFY,

    // Accelerator
    ASCII,
    ALT,
    // CONTROL,
    NOINVERT,
    SHIFT,
    VIRTKEY,

    // Statements
    CAPTION,
    CHARACTERISTICS,
    CLASS,
    EXSTYLE,
    // FONT,
    LANGUAGE,
    // MENU,
    MENUITEM,
    STYLE,
    VERSION,

    // Controls
    AUTO3STATE,
    AUTOCHECKBOX,
    AUTORADIOBUTTON,
    CHECKBOX,
    COMBOBOX,
    CONTROL,
    CTEXT,
    DEFPUSHBUTTON,
    EDITTEXT,
    GROUPBOX,
    // ICON,
    LISTBOX,
    LTEXT,
    PUSHBOX,
    PUSHBUTTON,
    RADIOBUTTON,
    RTEXT,
    SCROLLBAR,
    STATE3,
};

struct Token
{
    enum Type {
        Operator_Comma, // ,
        Operator_Or, // |
        Directive, // #...
        String, // "..."
        Integer, // [0-9]...
        Keyword, // See enum Keywords
        Word, // All the rest
    };
    Type type;
    std::variant<std::monostate, QString, int, Keywords> data;

    QString toString() const;
    int toInt() const { return std::get<int>(data); }
    Keywords toKeyword() const { return std::get<Keywords>(data); }

    QString prettyPrint() const;
};

class Lexer
{
public:
    Lexer(Stream input);

    void skipSpace();
    void skipLine();
    void skipComma();
    void skipScope();
    void skipToBegin();

    int line() const { return m_stream.line(); }
    QString content() const { return m_stream.content(); }

    void setFileName(const QString &name) { m_fileName = name; }
    QString fileName() const { return m_fileName; }

    std::optional<Token> next();
    std::optional<Token> peek();

    static QList<QString> keywords();

private:
    std::optional<Token> readNext();

    template <typename Func>
    void skipWhile(Func func)
    {
        while (!m_stream.atEnd() && std::invoke(func, m_stream.peek()))
            m_stream.next();
    }
    template <typename Func>
    QString readWhile(Func func)
    {
        QString result;
        while (!m_stream.atEnd() && std::invoke(func, m_stream.peek()))
            result += m_stream.next();
        return result;
    }

    Token readDirective();
    Token readString();
    Token readInclude();
    Token readNumber();
    Token readWord();

private:
    Stream m_stream;
    std::optional<Token> m_current;
    QString m_fileName;
};

} // namespace RcCore
