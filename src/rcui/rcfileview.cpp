/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "rcfileview.h"
#include "acceleratormodel.h"
#include "assetmodel.h"
#include "datamodel.h"
#include "dialogmodel.h"
#include "gui/highlightsearchdelegate.h"
#include "includemodel.h"
#include "menumodel.h"
#include "rccore/rcfile.h"
#include "rcviewer_global.h"
#include "ribbonmodel.h"
#include "stringmodel.h"
#include "toolbarmodel.h"
#include "ui_rcfileview.h"

#include <QBuffer>
#include <QClipboard>
#include <QMenu>
#include <QPushButton>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QTextBlock>
#include <QUiLoader>

namespace RcUi {

class DataProxy : public QSortFilterProxyModel
{
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        const bool isEmpty = sourceModel()->index(source_row, 0, source_parent).data(DataModel::EmptyRole).toBool();
        return isEmpty ? false : QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
};

RcFileView::RcFileView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RcFileView)
    , m_dataProxyModel(new DataProxy(this))
    , m_contentProxyModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    ui->dataView->setSortingEnabled(true);
    ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->dataView->setModel(m_dataProxyModel);
    ui->dataView->setItemDelegate(new Gui::HighlightSearchDelegate(this));
    connect(ui->dataView, &QTreeView::customContextMenuRequested, this, [this](const QPoint &pos) {
        slotContextMenu(ui->dataView, pos);
    });
    connect(ui->dataView->selectionModel(), &QItemSelectionModel::currentChanged, this, &RcFileView::changeDataItem);
    connect(ui->dataView->selectionModel(), &QItemSelectionModel::currentChanged, this, [this] {
        searchView(View::Content);
    });
    connect(ui->dataView, &QTreeView::doubleClicked, this, &RcFileView::previewData);
    connect(ui->dataSearch, &QLineEdit::textChanged, this, [this] {
        searchView(View::Data);
    });
    connect(ui->dataSearch, &QLineEdit::returnPressed, this, [this] {
        viewFindNext(View::Data);
    });
    connect(ui->dataFindPrevious, &QPushButton::clicked, this, [this] {
        viewFindPrevious(View::Data);
    });
    connect(ui->dataFindNext, &QPushButton::clicked, this, [this] {
        viewFindNext(View::Data);
    });

    m_dataProxyModel->setRecursiveFilteringEnabled(true);
    connect(ui->dataFilter, &QLineEdit::textChanged, m_dataProxyModel, &QSortFilterProxyModel::setFilterFixedString);

    ui->propertyView->setVisible(false);
    ui->propertyView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->propertyView, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        slotContextMenu(ui->propertyView, pos);
    });

    ui->contentView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->contentView->setModel(m_contentProxyModel);
    ui->contentView->setItemDelegate(new Gui::HighlightSearchDelegate(this));
    connect(ui->contentView, &QTreeView::customContextMenuRequested, this, [this](const QPoint &pos) {
        slotContextMenu(ui->contentView, pos);
    });

    connect(ui->contentSearch, &QLineEdit::textChanged, this, [this] {
        searchView(View::Content);
    });
    connect(ui->contentSearch, &QLineEdit::returnPressed, this, [this] {
        viewFindNext(View::Content);
    });
    connect(ui->contentFindPrevious, &QPushButton::clicked, this, [this] {
        viewFindPrevious(View::Content);
    });
    connect(ui->contentFindNext, &QPushButton::clicked, this, [this] {
        viewFindNext(View::Content);
    });

    m_contentProxyModel->setRecursiveFilteringEnabled(true);
    connect(ui->contentFilter, &QLineEdit::textChanged, m_contentProxyModel,
            &QSortFilterProxyModel::setFilterFixedString);

    connect(ui->searchText, &QLineEdit::textChanged, this, &RcFileView::slotSearchText);
    connect(ui->searchText, &QLineEdit::returnPressed, this, &RcFileView::slotSearchNext);

    auto findNext = new QShortcut(QKeySequence(QKeySequence::FindNext), this);
    findNext->setContext(Qt::WidgetWithChildrenShortcut);
    connect(findNext, &QShortcut::activated, this, &RcFileView::slotSearchNext);
    connect(ui->findNextText, &QPushButton::clicked, this, &RcFileView::slotSearchNext);

    auto findPrevious = new QShortcut(QKeySequence(QKeySequence::FindPrevious), this);
    findPrevious->setContext(Qt::WidgetWithChildrenShortcut);
    connect(findPrevious, &QShortcut::activated, this, &RcFileView::slotSearchPrevious);
    connect(ui->findPreviousText, &QPushButton::clicked, this, &RcFileView::slotSearchPrevious);

    connect(ui->languageCombo, &QComboBox::currentTextChanged, this, &RcFileView::languageChanged);
}

RcFileView::~RcFileView() = default;

void RcFileView::setRcFile(const RcCore::RcFile &rcFile)
{
    m_rcFile = &rcFile;

    auto languageList = m_rcFile->data.keys();
    std::ranges::sort(languageList);
    ui->languageCombo->clear();
    ui->languageCombo->addItems(languageList);

    delete m_dataProxyModel->sourceModel();
    auto model = new DataModel(rcFile, ui->languageCombo->currentText(), this);
    m_dataProxyModel->setSourceModel(model);
    ui->dataView->sortByColumn(0, Qt::AscendingOrder);

    delete m_contentModel;
    m_contentModel = nullptr;

    auto content = rcFile.content;
    ui->textEdit->setPlainText(content.replace("\t", "    "));

    connect(ui->languageCombo, &QComboBox::currentTextChanged, model, &DataModel::setLanguage);

    changeDataItem({});
}

QPlainTextEdit *RcFileView::textEdit() const
{
    return ui->textEdit;
}

void RcFileView::changeDataItem(const QModelIndex &current)
{
    int type = NoData;
    int index = -1;
    if (current.isValid()) {
        type = current.data(DataModel::TypeRole).toInt();
        index = current.data(DataModel::IndexRole).toInt();
        highlightLine(current.data(RcUi::LineRole).toInt());
    }
    setData(type, index);
}

void RcFileView::changeContentItem(const QModelIndex &current)
{
    if (current.isValid())
        highlightLine(current.data(RcUi::LineRole).toInt());
    else
        highlightLine(-1);
}

void RcFileView::setData(int type, int index)
{
    m_contentProxyModel->setSourceModel(nullptr);
    delete m_contentModel;
    m_contentModel = nullptr;
    ui->propertyView->setVisible(false);
    m_contentProxyModel->setFilterKeyColumn(0);

    switch (type) {
    case MenuData:
        if (index != -1)
            m_contentModel = new MenuModel(data(), index, this);
        break;
    case IconData:
        m_contentModel = new AssetModel(data().icons, this);
        break;
    case AssetData:
        m_contentModel = new AssetModel(data().assets, this);
        break;
    case StringData:
        m_contentModel = new StringModel(data(), this);
        break;
    case IncludeData:
        m_contentModel = new IncludeModel(*m_rcFile, this);
        break;
    case AcceleratorData:
        if (index != -1)
            m_contentModel = new AcceleratorModel(data(), index, this);
        break;
    case ToolBarData:
        if (index != -1)
            m_contentModel = new ToolBarModel(data(), index, this);
        break;
    case DialogData:
        if (index != -1) {
            m_contentModel = new DialogModel(data(), index, this);
            m_contentProxyModel->setFilterKeyColumn(1);
            updateDialogProperty(index);
        }
        break;
    case RibbonData:
        m_contentModel = new RibbonModel(data().ribbons, this);
        break;
    case NoData:
        break;
    }

    m_contentProxyModel->setSourceModel(m_contentModel);

    if (m_contentModel) {
        // Need to be done after setting the model
        ui->contentView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->contentView->expandAll();
        connect(ui->contentView->selectionModel(), &QItemSelectionModel::currentChanged, this,
                &RcFileView::changeContentItem);
    }
}

void RcFileView::updateDialogProperty(int index)
{
    const RcCore::Data::Dialog &dialog = data().dialogs.at(index);
    ui->propertyView->clear();
    ui->propertyView->setVisible(true);
    ui->propertyView->setHeaderLabels({tr("Property"), tr("Value")});
    ui->propertyView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    ui->propertyView->addTopLevelItem(new QTreeWidgetItem({tr("Id"), dialog.id}));
    const auto geometry = QStringLiteral("(%1, %2 %3x%4)")
                              .arg(dialog.geometry.x())
                              .arg(dialog.geometry.y())
                              .arg(dialog.geometry.width())
                              .arg(dialog.geometry.height());
    ui->propertyView->addTopLevelItem(new QTreeWidgetItem({tr("Geometry"), geometry}));
    ui->propertyView->addTopLevelItem(new QTreeWidgetItem({tr("Caption"), dialog.caption}));
    ui->propertyView->addTopLevelItem(new QTreeWidgetItem({tr("Menu"), dialog.menu}));
    ui->propertyView->addTopLevelItem(new QTreeWidgetItem({tr("Styles"), dialog.styles.join(QLatin1Char(','))}));
}

void RcFileView::previewData(const QModelIndex &index)
{
    // For now, we only supports dialog
    if (!index.isValid() || !index.parent().isValid() || index.data(DataModel::TypeRole) != DialogData)
        return;

    const int row = index.data(DataModel::IndexRole).toInt();
    const RcCore::Widget dialog =
        RcCore::convertDialog(data(), data().dialogs.value(row), RcCore::Widget::UpdateGeometry);
    QUiLoader loader;

    QBuffer buffer;
    if (buffer.open(QIODevice::WriteOnly)) {
        RcCore::writeDialogToUi(dialog, &buffer);
        buffer.close();
    }

    if (buffer.open(QIODevice::ReadOnly)) {
        QWidget *widget = loader.load(&buffer);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->show();
    }
}

void RcFileView::slotContextMenu(QTreeView *treeView, const QPoint &pos)
{
    QMenu menu(this);
    const QModelIndex index = treeView->indexAt(pos);
    menu.addAction(tr("Copy"), this, [index]() {
        const QString currentValue = index.data().toString();
        QClipboard *clip = QApplication::clipboard();
        clip->setText(currentValue, QClipboard::Clipboard);
        clip->setText(currentValue, QClipboard::Selection);
    });
    menu.exec(treeView->viewport()->mapToGlobal(pos));
}

void RcFileView::highlightLine(int line)
{
    if (line == -1) {
        ui->textEdit->setTextCursor({});
        return;
    }

    QTextCursor cursor(ui->textEdit->document()->findBlockByLineNumber(line - 1));
    cursor.select(QTextCursor::LineUnderCursor);
    ui->textEdit->setTextCursor(cursor);
}

void RcFileView::slotSearchText(const QString &text)
{
    const bool enable = !text.isEmpty();
    if (enable) {
        slotSearchNext();
    } else {
        // Clear the selection
        auto cursor = ui->textEdit->textCursor();
        cursor.clearSelection();
        ui->textEdit->setTextCursor(cursor);
    }
}

void RcFileView::slotSearchNext()
{
    const QString searchString = ui->searchText->text();
    ui->textEdit->find(searchString);
}

void RcFileView::slotSearchPrevious()
{
    const QString searchString = ui->searchText->text();
    ui->textEdit->find(searchString, QTextDocument::FindBackward);
}

const RcCore::Data &RcFileView::data() const
{
    Q_ASSERT(m_rcFile);
    return const_cast<RcCore::RcFile *>(m_rcFile)->data[ui->languageCombo->currentText()];
}

void RcFileView::searchView(View view)
{
    const ViewData data = viewData(view);
    QModelIndexList currentSearchResult;
    // Highlight results.
    // offset: The flat representation of the content treeView model displays its data indented with offset value.
    static_cast<Gui::HighlightSearchDelegate *>(data.treeView->itemDelegate())
        ->setSearchText(data.searchText, data.offset);
    // Update display.
    data.treeView->viewport()->update();
    // Search
    currentSearchResult = searchModel(view);
    if (view == View::Content) {
        // Store the search result
        m_viewSearchData.contentCurrentSearchResult = currentSearchResult;
        // Update the search text
        m_viewSearchData.contentCurrentSearchText = data.searchText;
        // Reset the search index
        m_viewSearchData.contentCurrentSearchIndex = 0;
    } else { // View::Data
        m_viewSearchData.dataCurrentSearchResult = currentSearchResult;
        m_viewSearchData.dataCurrentSearchText = data.searchText;
        m_viewSearchData.dataCurrentSearchIndex = 0;
    }

    // No hits
    if (currentSearchResult.isEmpty())
        return;

    if (data.searchText.isEmpty()) {
        // Do not select Empty results.
        data.treeView->clearSelection();
        return;
    }

    // Select the item that matched as the current index.
    data.treeView->selectionModel()->select(QItemSelection(currentSearchResult[0], currentSearchResult[0]),
                                            QItemSelectionModel::ClearAndSelect);
    data.treeView->scrollTo(currentSearchResult[0], QAbstractItemView::EnsureVisible);
    // Update view.
    if (view == View::Data)
        Q_EMIT data.treeView->selectionModel()->currentChanged(currentSearchResult[0], currentSearchResult[0]);
}

void RcFileView::viewFindPrevious(View view)
{
    const ViewData data = viewData(view);
    int index = data.currentSearchIndex - 1;
    if (data.currentSearchResult.isEmpty()) {
        searchView(view);
    } else {
        // If the index point to the first match continue with the last match.
        index = (index >= 0) ? index : data.currentSearchResult.count() - 1;
        // Select the Item
        data.treeView->selectionModel()->select(
            QItemSelection(data.currentSearchResult[index], data.currentSearchResult[index]),
            QItemSelectionModel::ClearAndSelect);
        data.treeView->scrollTo(data.currentSearchResult[index], QAbstractItemView::EnsureVisible);

        if (view == View::Content) {
            // Update current index
            m_viewSearchData.contentCurrentSearchIndex = index;
        } else { // Data view.
            // Update view.
            Q_EMIT ui->dataView->selectionModel()->currentChanged(data.currentSearchResult[index],
                                                                  data.currentSearchResult[index]);
            // Update current index
            m_viewSearchData.dataCurrentSearchIndex = index;
        }
    }
}

void RcFileView::viewFindNext(View view)
{
    const ViewData data = viewData(view);
    int index = data.currentSearchIndex + 1;
    if (data.currentSearchResult.isEmpty()) {
        searchView(view);
    } else {
        // if it is last match continue with the first match.
        index = (index < data.currentSearchResult.count()) ? index : 0;
        // Select the item
        data.treeView->selectionModel()->select(
            QItemSelection(data.currentSearchResult[index], data.currentSearchResult[index]),
            QItemSelectionModel::ClearAndSelect);
        data.treeView->scrollTo(data.currentSearchResult[index], QAbstractItemView::EnsureVisible);
        if (view == View::Content) {
            // Update current index
            m_viewSearchData.contentCurrentSearchIndex = index;
        } else { // Data view.
            // Update view.
            Q_EMIT ui->dataView->selectionModel()->currentChanged(data.currentSearchResult[index],
                                                                  data.currentSearchResult[index]);
            // Update current index
            m_viewSearchData.dataCurrentSearchIndex = index;
        }
    }
}

RcFileView::ViewData RcFileView::viewData(View view)
{
    QTreeView *treeView = nullptr;
    QString searchText;
    QModelIndexList currentSearchResult;
    int currentSearchIndex = 0;
    int offset = 0;

    switch (view) {
    case View::Content:
        treeView = ui->contentView;
        searchText = ui->contentSearch->text();
        currentSearchResult = m_viewSearchData.contentCurrentSearchResult;
        currentSearchIndex = m_viewSearchData.contentCurrentSearchIndex;
        offset = 3;
        break;
    case View::Data:
        treeView = ui->dataView;
        searchText = ui->dataSearch->text();
        currentSearchResult = m_viewSearchData.dataCurrentSearchResult;
        currentSearchIndex = m_viewSearchData.dataCurrentSearchIndex;
        offset = 3;
    }

    Q_ASSERT(treeView);

    return ViewData {treeView, searchText, offset, currentSearchResult, currentSearchIndex};
}

QModelIndexList RcFileView::searchModel(const View &view) const
{
    const QAbstractItemModel *model = nullptr;
    QString searchText;

    switch (view) {
    case View::Content:
        model = ui->contentView->model();
        searchText = ui->contentSearch->text();
        break;
    case View::Data:
        model = ui->dataView->model();
        searchText = ui->dataSearch->text();
    }

    Q_ASSERT(model);

    // Retrieve and store the indexes which match the search (root and child indexes).
    QModelIndexList searchResults;
    for (int row = 0; row < model->rowCount(); ++row) {
        const QModelIndex rootIndex = model->index(row, 0);
        if (hasMatch(rootIndex, searchText)) {
            searchResults.append(rootIndex);
        }

        // Iterate through the child indexes (tree model)
        for (int childRow = 0; childRow < model->rowCount(rootIndex); ++childRow) {
            for (int column = 0; column < model->columnCount(); ++column) {
                const QModelIndex childIndex = model->index(childRow, column, rootIndex);
                if (hasMatch(childIndex, searchText)) {
                    searchResults.append(childIndex);
                }
            }
        }
        // Iterarate through the columns (flat model)
        for (int column = 0; column < model->columnCount(); ++column) {
            const QModelIndex index = model->index(row, column);
            if (!searchResults.contains(index) && hasMatch(index, searchText)) {
                searchResults.append(index);
            }
        }
    }
    return searchResults;
}

bool RcFileView::hasMatch(const QModelIndex &index, const QString &searchText) const
{
    if (index.isValid()) {
        const QString data = index.data().toString();
        if ((searchText.isEmpty() && data == searchText)
            || (!searchText.isEmpty() && data.contains(searchText, Qt::CaseInsensitive))) {
            return true;
        }
    }
    return false;
}

} // namespace RcUi
