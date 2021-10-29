#include "fileselector.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>

namespace Gui {

FileSelector::FileSelector(QWidget *parent)
    : QWidget(parent)
    , m_lineEdit(new QLineEdit(this))
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(m_lineEdit);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &FileSelector::fileNameChanged);

    auto button = new QToolButton(this);
    button->setText(QStringLiteral("..."));
    layout->addWidget(button);
    connect(button, &QToolButton::clicked, this, &FileSelector::chooseFile);

    setFocusProxy(m_lineEdit);
}

QString FileSelector::fileName() const
{
    return m_lineEdit->text();
}

FileSelector::Mode FileSelector::mode() const
{
    return m_mode;
}

void FileSelector::setFilter(const QString &filter)
{
    m_filter = filter;
}

void FileSelector::setFileName(const QString &fileName)
{
    m_lineEdit->setText(fileName);
}

void FileSelector::setMode(FileSelector::Mode mode)
{
    m_mode = mode;
}

void FileSelector::chooseFile()
{
    QString fileName;
    switch (m_mode) {
    case Mode::OpenFile:
        fileName = QFileDialog::getOpenFileName(this, QString(), m_lineEdit->text(), m_filter);
        break;
    case Mode::OpenDirectory:
        fileName = QFileDialog::getExistingDirectory(this, QString(), m_lineEdit->text());
        break;
    case Mode::SaveFile:
        fileName = QFileDialog::getSaveFileName(this, QString(), m_lineEdit->text(), m_filter);
        break;
    }

    if (!fileName.isEmpty())
        m_lineEdit->setText(fileName);
}

} // namespace Gui
