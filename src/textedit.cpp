#include "textedit.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QShortcut>
#include <QLabel>

TextEdit::TextEdit(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);
    mainLayout->addWidget(m_textEdit);

    m_searchWidget = new QWidget(this);
    mainLayout->addWidget(m_searchWidget);

    QHBoxLayout *searchLayout = new QHBoxLayout(m_searchWidget);
    searchLayout->setContentsMargins(0, 0, 0, 0);


    QLabel *label = new QLabel(QStringLiteral("Search:"), this);
    searchLayout->addWidget(label);

    m_searchText = new QLineEdit(this);
    m_searchText->setClearButtonEnabled(true);
    searchLayout->addWidget(m_searchText);

    m_searchPreviewButton = new QPushButton(QStringLiteral("Previous"), this);
    searchLayout->addWidget(m_searchPreviewButton);

    m_searchNextButton = new QPushButton(QStringLiteral("Next"), this);
    searchLayout->addWidget(m_searchNextButton);

    connect(m_searchText, &QLineEdit::textChanged, this, &TextEdit::slotSearchLineEditChanged);
    connect(m_searchPreviewButton, &QPushButton::clicked, this, &TextEdit::slotSearchPreviewText);
    connect(m_searchNextButton, &QPushButton::clicked, this, &TextEdit::slotSearchNextText);
    slotSearchLineEditChanged(QString());
    m_searchWidget->setVisible(false);

    QShortcut *findText = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), m_textEdit);
    findText->setContext(Qt::WidgetShortcut);
    connect(findText, &QShortcut::activated, this, &TextEdit::slotSearchText);

    QShortcut *escape = new QShortcut(QKeySequence(Qt::Key_Escape), m_textEdit);
    escape->setContext(Qt::WidgetShortcut);
    connect(escape, &QShortcut::activated, this, &TextEdit::slotCloseSearchText);
}

TextEdit::~TextEdit()
{
}

void TextEdit::slotSearchLineEditChanged(const QString &str)
{
    const bool enable = !str.isEmpty();
    m_searchPreviewButton->setEnabled(enable);
    m_searchNextButton->setEnabled(enable);
}

void TextEdit::slotSearchPreviewText()
{
    const QString searchString = m_searchText->text();
    m_textEdit->find(searchString, QTextDocument::FindBackward);
}

void TextEdit::slotSearchNextText()
{
    const QString searchString = m_searchText->text();
    m_textEdit->find(searchString);
}

QTextEdit *TextEdit::textEdit() const
{
    return m_textEdit;
}

void TextEdit::slotSearchText()
{
    if (m_searchWidget->isVisible()) {
        return;
    }
    m_searchWidget->setVisible(true);
}

void TextEdit::slotCloseSearchText()
{
    if (!m_searchWidget->isVisible()) {
        return;
    }
    m_searchWidget->setVisible(false);
}
