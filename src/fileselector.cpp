#include "fileselector.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>

FileSelector::FileSelector(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);

    m_lineEdit = new QLineEdit(this);
    layout->addWidget(m_lineEdit);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &FileSelector::fileNameChanged);

    QToolButton *button = new QToolButton(this);
    button->setText(QStringLiteral("..."));
    layout->addWidget(button);
    connect(button, &QToolButton::clicked, this, &FileSelector::chooseFile);

    setFocusProxy(m_lineEdit);
}

QString FileSelector::fileName() const
{
    return m_lineEdit->text();
}

void FileSelector::setFilter(const QString &filter)
{
    m_filter = filter;
}

void FileSelector::setFileName(const QString &fileName)
{
    m_lineEdit->setText(fileName);
}

void FileSelector::chooseFile()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this, QString(), m_lineEdit->text(), m_filter);

    if (!fileName.isEmpty())
        m_lineEdit->setText(fileName);
}
