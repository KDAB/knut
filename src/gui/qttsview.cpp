/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qttsview.h"
#include "findwidget.h"
#include "highlightsearchdelegate.h"

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>

namespace Gui {

class QtTsModelView : public QAbstractTableModel
{
public:
    enum TsColumn { Context = 0, Source, Translation, Comment };

    QtTsModelView(Core::QtTsDocument *document)
        : QAbstractTableModel(document)
        , m_document(document)
    {
        Q_ASSERT(document);
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return m_document->messages().count();
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return 4;
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole) {
            auto widget = m_document->messages().at(index.row());
            switch (index.column()) {
            case TsColumn::Comment:
                return widget->comment();
            case TsColumn::Source:
                return widget->source();
            case TsColumn::Translation:
                return widget->translation();
            case TsColumn::Context:
                return widget->context();
            }
        }
        return {};
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case TsColumn::Comment:
                return tr("Comment");
            case TsColumn::Source:
                return tr("Source");
            case TsColumn::Translation:
                return tr("Translation");
            case TsColumn::Context:
                return tr("Context");
            }
        }
        return QAbstractItemModel::headerData(section, orientation, role);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if (role != Qt::EditRole)
            return false;
        auto widget = m_document->messages().at(index.row());
        switch (index.column()) {
        case TsColumn::Comment:
            widget->setComment(value.toString());
            break;
        case TsColumn::Source:
            widget->setSource(value.toString());
            break;
        case TsColumn::Translation:
            widget->setTranslation(value.toString());
            break;
        }
        return true;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (index.column() == TsColumn::Translation) {
            return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        }
        return QAbstractTableModel::flags(index);
    }

private:
    Core::QtTsDocument *const m_document;
};

class QtTsProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    void setFilterText(const QString &str);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_filterText;
};

QtTsView::QtTsView(QWidget *parent)
    : QWidget(parent)
    , m_tableView(new QTableView(this))
    , m_searchLineEdit(new QLineEdit(this))
    , m_contentProxyModel(new QtTsProxy(this))
{
    auto mainWidgetLayout = new QVBoxLayout(this);
    m_tableView->verticalHeader()->hide();
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Set the delegate
    m_tableView->setItemDelegate(new HighlightSearchDelegate(this));
    mainWidgetLayout->addWidget(m_tableView);
    mainWidgetLayout->addWidget(m_searchLineEdit);
    m_searchLineEdit->setPlaceholderText(tr("Filter..."));
    m_searchLineEdit->setClearButtonEnabled(true);

    m_contentProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_tableView->setModel(m_contentProxyModel);
    connect(m_searchLineEdit, &QLineEdit::textChanged, m_contentProxyModel, &QtTsProxy::setFilterText);
}

void QtTsView::setTsDocument(Core::QtTsDocument *document)
{
    Q_ASSERT(document);

    if (m_document)
        m_document->disconnect(this);

    m_document = document;
    if (m_document) {
        connect(m_document, &Core::QtTsDocument::fileUpdated, this, &QtTsView::updateView);
    }
    updateView();
}

void QtTsView::setFindWidget(FindWidget *findWidget)
{
    Q_ASSERT(findWidget);

    // Connections
    connect(findWidget, &FindWidget::findRequested, this, &QtTsView::search);

    auto updateDisplay = [this]() {
        search(""); // Reset delegate highlighting
        m_tableView->viewport()->update();
        m_tableView->clearSelection();
    };
    connect(findWidget, &FindWidget::findWidgetClosed, this, updateDisplay);
}

void QtTsView::updateView()
{
    m_contentProxyModel->setSourceModel(nullptr);
    delete m_contentModel;
    m_contentModel = new QtTsModelView(m_document);

    m_contentProxyModel->setSourceModel(m_contentModel);
    m_tableView->setModel(m_contentProxyModel);
    m_tableView->horizontalHeader()->resizeSection(QtTsModelView::TsColumn::Source, 200);
    m_tableView->horizontalHeader()->resizeSection(QtTsModelView::TsColumn::Translation, 200);
    m_tableView->horizontalHeader()->resizeSection(QtTsModelView::TsColumn::Context, 200);
    m_tableView->horizontalHeader()->setSectionResizeMode(QtTsModelView::TsColumn::Source, QHeaderView::Fixed);
    m_tableView->horizontalHeader()->setSectionResizeMode(QtTsModelView::TsColumn::Translation, QHeaderView::Fixed);
    m_tableView->horizontalHeader()->setSectionResizeMode(QtTsModelView::TsColumn::Context, QHeaderView::Interactive);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
}

void QtTsView::search(const QString &searchText, int options)
{
    // Retrieve the text margin for the text inside a cell depending on the style.
    QStyle *widgetStyle = style();
    // Ref: See textMargin - QItemDelegate::drawDisplay.
    const int textMargin = widgetStyle->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

    // Search only in case the search was not already processed (inclusive empty string search).
    if (searchText != m_currentSearchText || !m_initialSearchProcessed) {
        // Colorize search hits (This gives an overview over the search result)
        // textMargin: Displays the highlighted data indented with the style text margin value.
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
                // if it is the last match continue with the first match.
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

QModelIndexList QtTsView::searchModel(const QString &searchText, const QAbstractItemModel *model) const
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

void QtTsProxy::setFilterText(const QString &str)
{
    m_filterText = str;
    invalidateFilter();
}

bool QtTsProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    auto match = [&](int role) {
        return m_filterText.isEmpty()
            || sourceModel()
                   ->index(source_row, role, source_parent)
                   .data()
                   .toString()
                   .contains(m_filterText, Qt::CaseInsensitive);
    };
    if (!match(QtTsModelView::TsColumn::Comment) && !match(QtTsModelView::TsColumn::Source)
        && !match(QtTsModelView::TsColumn::Translation) && !match(QtTsModelView::TsColumn::Context)) {
        return false;
    }

    return true;
}
}
#include "qttsview.moc"
