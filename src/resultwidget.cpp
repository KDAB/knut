#include "resultwidget.h"

#include <QTextEdit>

ResultWidget::ResultWidget(QWidget *parent)
    : QTabWidget(parent)
{
}

void ResultWidget::setResult(const QVector<JsResult::Document> &documents)
{
    clear();
    for (const auto &doc : documents) {
        auto edit = new QTextEdit(this);
        edit->setReadOnly(true);
        edit->setText(doc.content);
        addTab(edit, doc.title);
    }
}
