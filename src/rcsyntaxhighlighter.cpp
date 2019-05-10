#include "rcsyntaxhighlighter.h"

RcSyntaxHighlighter::RcSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    const QStringList keywordPatterns = {
        "ACCELERATORS",
        "AFX_DIALOG_LAYOUT",
        "ALT",
        "ASCII",
        "AUTO3STATE",
        "AUTOCHECKBOX",
        "AUTORADIOBUTTON",
        "BEGIN",
        "BITMAP",
        "BUTTON",
        "CAPTION",
        "CHARACTERISTICS",
        "CHECKBOX",
        "CLASS",
        "COMBOBOX",
        "CONTROL",
        "CTEXT",
        "CURSOR",
        "DEFPUSHBUTTON",
        "DESIGNINFO",
        "DIALOG",
        "DIALOGEX",
        "DLGINIT",
        "EDITTEXT",
        "END",
        "EXSTYLE",
        "FONT",
        "GROUPBOX",
        "HTML",
        "ICON",
        "LANGUAGE",
        "LISTBOX",
        "LTEXT",
        "MENU",
        "MENUEX",
        "MENUITEM",
        "MESSAGETABLE",
        "NOT",
        "PNG",
        "POPUP",
        "PUSHBOX",
        "PUSHBUTTON",
        "RADIOBUTTON",
        "RCDATA",
        "RTEXT",
        "SCROLLBAR",
        "SEPARATOR",
        "SHIFT",
        "STATE3",
        "STRINGTABLE",
        "STYLE",
        "TEXTINCLUDE",
        "TOOLBAR",
        "VERSION",
        "VERSIONINFO",
    };

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::blue);

    for (const auto &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression("\\b" + pattern + "\\b");
        rule.format = keywordFormat;
        m_highlightingRules.append(rule);
    }

    QTextCharFormat numberFormat;
    numberFormat.setForeground(Qt::magenta);
    rule.pattern = QRegularExpression(R"([-+]?\d+)");
    rule.format = numberFormat;
    m_highlightingRules.append(rule);

    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkGray);
    rule.pattern = QRegularExpression(R"("[^"\\]*(\\.[^"\\]*)*")");
    rule.format = quotationFormat;
    m_highlightingRules.append(rule);

    m_commentFormat.setForeground(Qt::darkGreen);
}

void RcSyntaxHighlighter::highlightBlock(const QString &text)
{
    if (text.startsWith("//")) {
        setFormat(0, text.size(), m_commentFormat);
    } else {
        for (const auto &rule : m_highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }
}
