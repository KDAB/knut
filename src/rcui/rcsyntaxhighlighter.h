#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QVector>

namespace RcUi {

class RcSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit RcSyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> m_highlightingRules;

    QTextCharFormat m_commentFormat;
};

} // namespace RcUi
