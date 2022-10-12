#ifndef TREESITTERINSPECTOR_H
#define TREESITTERINSPECTOR_H

#include "treesittertreemodel.h"

#include <QDialog>
#include <QSyntaxHighlighter>

#include <treesitter/parser.h>

namespace treesitter {
class Transformation;
}

namespace Core {
class LspDocument;
}

namespace Gui {
namespace Ui {
    class TreeSitterInspector;
}

class QueryErrorHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    QueryErrorHighlighter(QTextDocument *parent);

    void highlightBlock(const QString &text) override;

    void setUtf8Position(int position);

private:
    int m_errorUtf8Position = -1;
};

class TreeSitterInspector : public QDialog
{
    Q_OBJECT

public:
    explicit TreeSitterInspector(QWidget *parent = nullptr);
    ~TreeSitterInspector();

private:
    void currentDocumentChanged();
    void setDocument(Core::LspDocument *document);
    void textChanged();
    void cursorChanged();
    void queryChanged();
    void queryStateChanged();
    void previewTransformation();
    void runTransformation();
    void prepareTransformation(std::function<void(treesitter::Transformation &transformation)> runFunction);

    QString preCheckTransformation() const;

    void treeSelectionChanged(const QModelIndex &current, const QModelIndex &previous);

    QString highlightQueryError(const treesitter::Query::Error &error) const;

    Ui::TreeSitterInspector *ui;

    treesitter::Parser m_parser;
    TreeSitterTreeModel m_treemodel;
    QueryErrorHighlighter *m_errorHighlighter;

    Core::LspDocument *m_document;

    QString m_queryText;
};

} // namespace Gui

#endif // TREESITTERINSPECTOR_H
