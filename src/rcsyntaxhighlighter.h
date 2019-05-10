#ifndef RCSYNTAXHIGHLIGHTER_H
#define RCSYNTAXHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QVector>

class RcSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit RcSyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> m_highlightingRules;

    QTextCharFormat m_commentFormat;
};

#endif // RCSYNTAXHIGHLIGHTER_H
