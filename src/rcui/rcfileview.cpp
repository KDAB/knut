#include "rcfileview.h"
#include "ui_rcfileview.h"

#include "acceleratormodel.h"
#include "assetmodel.h"
#include "datamodel.h"
#include "dialogmodel.h"
#include "includemodel.h"
#include "menumodel.h"
#include "rcviewer_global.h"
#include "stringmodel.h"
#include "toolbarmodel.h"

#include "rccore/data.h"
#include "rccore/rcfile.h"

#include <QBuffer>
#include <QClipboard>
#include <QMenu>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QTextBlock>
#include <QUiLoader>

namespace RcUi {

RcFileView::RcFileView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RcFileView)
    , m_dataProxyModel(new QSortFilterProxyModel(this))
    , m_contentProxyModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    ui->dataView->setSortingEnabled(true);
    ui->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->dataView->setModel(m_dataProxyModel);
    connect(ui->dataView, &QTreeView::customContextMenuRequested, this, [this](const QPoint &pos) {
        slotContextMenu(ui->dataView, pos);
    });
    connect(ui->dataView->selectionModel(), &QItemSelectionModel::currentChanged, this, &RcFileView::changeDataItem);
    connect(ui->dataView, &QTreeView::doubleClicked, this, &RcFileView::previewData);

    m_dataProxyModel->setRecursiveFilteringEnabled(true);
    connect(ui->dataFilter, &QLineEdit::textChanged, m_dataProxyModel, &QSortFilterProxyModel::setFilterFixedString);

    ui->propertyView->setVisible(false);
    ui->propertyView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->propertyView, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        slotContextMenu(ui->propertyView, pos);
    });

    ui->contentView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->contentView->setModel(m_contentProxyModel);
    connect(ui->contentView, &QTreeView::customContextMenuRequested, this, [this](const QPoint &pos) {
        slotContextMenu(ui->contentView, pos);
    });

    m_contentProxyModel->setRecursiveFilteringEnabled(true);
    connect(ui->contentFilter, &QLineEdit::textChanged, m_contentProxyModel,
            &QSortFilterProxyModel::setFilterFixedString);

    connect(ui->searchText, &QLineEdit::textChanged, this, &RcFileView::slotSearchText);
    connect(ui->searchText, &QLineEdit::returnPressed, this, &RcFileView::slotSearchNext);

    QShortcut *findNext = new QShortcut(QKeySequence(QKeySequence::FindNext), this);
    findNext->setContext(Qt::WidgetWithChildrenShortcut);
    connect(findNext, &QShortcut::activated, this, &RcFileView::slotSearchNext);

    QShortcut *findPrevious = new QShortcut(QKeySequence(QKeySequence::FindPrevious), this);
    findPrevious->setContext(Qt::WidgetWithChildrenShortcut);
    connect(findPrevious, &QShortcut::activated, this, &RcFileView::slotSearchPrevious);
}

RcFileView::~RcFileView() = default;

void RcFileView::setRcFile(const RcCore::Data &data)
{
    m_data = &data;
    auto model = new DataModel(data, this);
    m_dataProxyModel->setSourceModel(model);
    ui->dataView->sortByColumn(0, Qt::AscendingOrder);

    delete m_contentModel;
    m_contentModel = nullptr;

    auto content = data.content;
    ui->textEdit->setPlainText(content.replace("\t", "    "));
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
        m_contentModel = new IncludeModel(data(), this);
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
    Q_ASSERT(m_data);
    return *m_data;
}

} // namespace RcUi
