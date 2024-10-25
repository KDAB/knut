/*
  This file is part of Knut.

SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

SPDX-License-Identifier: GPL-3.0-only

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "findinfilespanel.h"
#include "core/project.h"
#include "core/textdocument.h"
#include <QFile>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPainter>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace Gui {

enum { LineRole = Qt::UserRole + 1, ColumnRole };

class FindInFilesTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit FindInFilesTreeWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    const bool m_findInFilesAvailable;
};

FindInFilesPanel::FindInFilesPanel(QWidget *parent)
    : QWidget(parent)
    , m_toolBar(new QWidget(this))
    , m_resultsDisplay(new FindInFilesTreeWidget(this))
{
    setWindowTitle(tr("Find in Files"));
    setObjectName("FindInFilesPanel");

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setupToolBar();
    mainLayout->addWidget(m_toolBar);

    m_resultsDisplay->setHeaderHidden(true);
    m_resultsDisplay->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    mainLayout->addWidget(m_resultsDisplay);

    connect(m_resultsDisplay, &QTreeWidget::itemActivated, this, [this](QTreeWidgetItem *item, int) {
        openFileAtItem(item);
    });
    const bool available = Core::Project::instance()->isFindInFilesAvailable();
    m_searchInput->setEnabled(available);
}

QWidget *FindInFilesPanel::toolBar() const
{
    return m_toolBar;
}

void FindInFilesPanel::setupToolBar()
{
    auto layout = new QHBoxLayout(m_toolBar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_searchInput = new QLineEdit(m_toolBar);
    m_searchInput->setPlaceholderText(tr("Enter search pattern..."));
    layout->addWidget(m_searchInput);

    QToolButton *searchButton = new QToolButton(m_toolBar);
    searchButton->setText(tr("Find"));
    searchButton->setEnabled(false);
    layout->addWidget(searchButton);

    connect(m_searchInput, &QLineEdit::textChanged, this, [searchButton, this]() {
        searchButton->setEnabled(!m_searchInput->text().isEmpty());
    });

    connect(searchButton, &QToolButton::clicked, this, &FindInFilesPanel::findInFiles);
    connect(m_searchInput, &QLineEdit::returnPressed, this, &FindInFilesPanel::findInFiles);
}

void FindInFilesPanel::findInFiles()
{
    auto results = Core::Project::instance()->findInFiles(m_searchInput->text());

    for (auto &resultVariant : results) {
        auto result = resultVariant.toMap();

        QFile file(result["file"].toString());
        if (file.open(QFile::ReadOnly)) {
            QTextStream stream(&file);
            int currentLine = 0;
            const int line = result["line"].toInt();
            while (!stream.atEnd()) {
                const QString lineText = stream.readLine();
                ++currentLine;
                if (currentLine == line) {
                    result["text"] = lineText;
                    break;
                }
            }
        }
        resultVariant = result;
    }

    displayResults(results);
}

void FindInFilesPanel::displayResults(const QVariantList &results) const
{
    m_resultsDisplay->clear();
    QMap<QString, QTreeWidgetItem *> fileItems;

    for (const auto &result : results) {
        const auto map = result.toMap();
        const QString filePath = map["file"].toString();
        const int line = map["line"].toInt();
        const int column = map["column"].toInt();
        const QString text = map["text"].toString();

        auto it = fileItems.find(filePath);
        if (it == fileItems.end()) {
            auto fileItem = new QTreeWidgetItem(m_resultsDisplay);
            fileItem->setText(0, filePath);
            it = fileItems.insert(filePath, fileItem);
        }

        auto lineItem = new QTreeWidgetItem(it.value());
        lineItem->setText(0, QString("%1        %2").arg(line).arg(text));
        lineItem->setData(0, LineRole, line);
        lineItem->setData(0, ColumnRole, column);
    }
}

void FindInFilesPanel::openFileAtItem(QTreeWidgetItem *item)
{
    if (item->childCount() > 0)
        return;

    QTreeWidgetItem *parentItem = item->parent();
    const QString filePath = parentItem->text(0);
    const int line = item->data(0, LineRole).toInt();
    const int column = item->data(0, ColumnRole).toInt();

    if (auto *doc = qobject_cast<Core::TextDocument *>(Core::Project::instance()->open(filePath))) {
        doc->gotoLine(line, column);
        doc->selectEndOfWord();
    } else {
        qWarning() << "Unable to open the file:" << filePath;
    }
}

FindInFilesTreeWidget::FindInFilesTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_findInFilesAvailable(Core::Project::instance()->isFindInFilesAvailable())
{
}

void FindInFilesTreeWidget::paintEvent(QPaintEvent *event)
{
    if (!m_findInFilesAvailable) {
        QPainter p(viewport());

        QFont font = p.font();
        font.setItalic(true);
        p.setFont(font);

        p.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter,
                   tr("Ripgrep (rg) executable not found.\nPlease ensure that ripgrep is installed and its location is "
                      "included in the PATH environment variable."));
    } else {
        QTreeWidget::paintEvent(event);
    }
}

} // namespace Gui
#include "findinfilespanel.moc"
