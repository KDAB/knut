#include "rcsyntaxhighlighter.h"

#include "rccore/lexer.h"

namespace RcUi {

RcSyntaxHighlighter::RcSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::blue);

    const auto keywords = RcCore::Lexer::keywords();
    for (const auto &pattern : keywords) {
        rule.pattern = QRegularExpression("\\b" + pattern + "\\b");
        rule.format = keywordFormat;
        m_highlightingRules.append(rule);
    }

    QTextCharFormat numberFormat;
    numberFormat.setForeground(Qt::magenta);
    rule.pattern = QRegularExpression(R"(\b-?\d+\b)");
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
        for (const auto &rule : qAsConst(m_highlightingRules)) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }
}

} // namespace RcUi
