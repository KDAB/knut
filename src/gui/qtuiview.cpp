/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qtuiview.h"
#include "core/qtuidocument.h"
#include "findwidget.h"
#include "highlightsearchdelegate.h"

#include <QAbstractTableModel>
#include <QFile>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QTableView>
#include <QUiLoader>

namespace Gui {

class QtUiModelView : public QAbstractTableModel
{
public:
    enum QtUiColumn {
        Name = 0,
        ClassName,
    };

    QtUiModelView(Core::QtUiDocument *document)
        : QAbstractTableModel(document)
        , m_document(document)
    {
        Q_ASSERT(document);
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return m_document->widgets().count();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return 2;
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            auto widget = m_document->widgets().at(index.row());
            switch (index.column()) {
            case QtUiColumn::Name:
                return widget->name();
            case QtUiColumn::ClassName:
                return widget->className();
            }
        }
        return {};
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case QtUiColumn::Name:
                return tr("Name");
            case QtUiColumn::ClassName:
                return tr("Class Name");
            }
        }
        return QAbstractItemModel::headerData(section, orientation, role);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if (role != Qt::EditRole)
            return false;
        auto widget = m_document->widgets().at(index.row());
        switch (index.column()) {
        case QtUiColumn::Name:
            widget->setName(value.toString());
            break;
        case QtUiColumn::ClassName:
            widget->setClassName(value.toString());
            break;
        }
        return true;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }

private:
    Core::QtUiDocument *m_document = nullptr;
};

QtUiView::QtUiView(QWidget *parent)
    : QSplitter(parent)
    , m_tableView(new QTableView(this))
    , m_previewArea(new QMdiArea(this))
{
    addWidget(m_previewArea);
    addWidget(m_tableView);

    m_tableView->verticalHeader()->hide();
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_previewArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_previewArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Set the delegate
    m_tableView->setItemDelegate(new HighlightSearchDelegate(this));
}

void QtUiView::search(const QString &searchText, int options)
{
    // Retrieve the left margin for the text inside a cell depending on the style.
    QStyle *widgetStyle = style();
    // Ref: See textMargin - QItemDelegate::drawDisplay.
    const int textMargin = widgetStyle->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

    // Search only in case the search was not already processed (inclusive empty string search).
    if (searchText != m_currentSearchText || !m_initialSearchProcessed) {
        // Colorize search hits (This gives an overview over the search result)
        // textMargin: Displays the highlighted data indented with the style left text margin value.
        static_cast<HighlightSearchDelegate *>(m_tableView->itemDelegate())->setSearchText(searchText, textMargin);
        // Update display.
        m_tableView->viewport()->update();
        // Search...
        m_currentSearchResult = searchModel(searchText, m_tableView->model());
        // Reset the search index.
        m_currentSearchIndex = 0;
        m_initialSearchProcessed = true;
    } else { // Search was already processed. Handle options (Backward, Forward)
        if (!m_currentSearchResult.isEmpty()) {
            int index;
            switch (options) {
            case Core::TextDocument::FindBackward: {
                index = m_currentSearchIndex - 1;
                // if it is the first match continue with the last match.
                index = (index >= 0) ? index : m_currentSearchResult.count() - 1;
                break;
            }
            default: { // Default is FindForward
                index = m_currentSearchIndex + 1;
                // if it is last match continue with the first match.
                index = (index < m_currentSearchResult.count()) ? index : 0;
                break;
            }
            }
            // Update the current search index
            m_currentSearchIndex = index;
        }
    }
    // Update the current search text
    m_currentSearchText = searchText;
    // Select (highlight) the cell that matched, as the current index.
    if (!m_currentSearchResult.isEmpty()) {
        m_tableView->selectionModel()->setCurrentIndex(m_currentSearchResult[m_currentSearchIndex],
                                                       QItemSelectionModel::SelectCurrent);
    }
}

QModelIndexList QtUiView::searchModel(const QString &searchText, const QAbstractItemModel *model) const
{
    QModelIndexList searchResults;
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int column = 0; column < model->columnCount(); ++column) {
            const QModelIndex index = model->index(row, column);
            const QString data = model->data(index).toString();
            if ((searchText.isEmpty() && data == searchText)
                || (!searchText.isEmpty() && data.contains(searchText, Qt::CaseInsensitive))) {
                searchResults.append(index);
            }
        }
    }
    return searchResults;
}

void QtUiView::setUiDocument(Core::QtUiDocument *document)
{
    Q_ASSERT(document);

    if (m_document)
        m_document->disconnect(this);

    m_document = document;
    if (m_document) {
        connect(m_document, &Core::QtUiDocument::fileUpdated, this, &QtUiView::updateView);
    }
    updateView();
}

void QtUiView::setFindWidget(FindWidget *findWidget)
{
    Q_ASSERT(findWidget);

    connect(findWidget, &FindWidget::findRequested, this, &QtUiView::search);

    // Update display.
    auto updateDisplay = [this]() {
        search(""); // Reset delegate highlighting
        m_tableView->viewport()->update();
        m_tableView->clearSelection();
    };
    connect(findWidget, &FindWidget::findWidgetClosed, this, updateDisplay);
}

void QtUiView::updateView()
{
    delete m_tableView->model();
    m_previewArea->removeSubWindow(m_previewWindow);

    m_tableView->setModel(new QtUiModelView(m_document));
    m_tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    m_tableView->setMinimumWidth(m_tableView->horizontalHeader()->sectionSize(QtUiModelView::QtUiColumn::Name)
                                 + m_tableView->horizontalHeader()->sectionSize(QtUiModelView::QtUiColumn::ClassName)
                                 + 2 * m_tableView->frameWidth());

    QUiLoader loader;
    QFile file(m_document->fileName());
    if (file.open(QIODevice::ReadOnly)) {
        QWidget *widget = loader.load(&file);
        if (!widget) {
            QMessageBox::warning(this, tr("Knut Ui View"),
                                 tr("Can't load the ui file due to some errors:\n%1").arg(loader.errorString()));
            return;
        }
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->setMinimumSize(widget->size());
        m_previewWindow = m_previewArea->addSubWindow(widget, Qt::CustomizeWindowHint);
        m_previewWindow->setVisible(true);
    }
}

} // namespace Gui
