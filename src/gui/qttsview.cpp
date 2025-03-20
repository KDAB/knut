/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qttsview.h"
#include "core/textdocument.h"
#include "findadapter.h"

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>

namespace Gui {

//=============================================================================
// Model with all the translations
//=============================================================================
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
        if (parent.isValid())
            return 0;
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

//=============================================================================
// Proxy model used to filter data
//=============================================================================
class QtTsProxy : public QSortFilterProxyModel
{
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    void setFilterText(const QString &str)
    {
        m_filterText = str;
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
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

private:
    QString m_filterText;
};

//=============================================================================
// QtTsView
//=============================================================================
QtTsView::QtTsView(QWidget *parent)
    : QWidget(parent)
    , FindInterface(FindInterface::CanSearch)
    , m_tableView(new QTableView(this))
    , m_findAdapter(std::make_unique<FindAdapter>(m_tableView))
    , m_searchLineEdit(new QLineEdit(this))
    , m_contentProxyModel(new QtTsProxy(this))
{
    auto mainWidgetLayout = new QVBoxLayout(this);
    m_tableView->verticalHeader()->hide();
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_findAdapter->updateItemDelegate();
    mainWidgetLayout->addWidget(m_tableView);
    mainWidgetLayout->addWidget(m_searchLineEdit);
    m_searchLineEdit->setPlaceholderText(tr("Filter..."));
    m_searchLineEdit->setClearButtonEnabled(true);

    m_contentProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_tableView->setModel(m_contentProxyModel);
    connect(m_searchLineEdit, &QLineEdit::textChanged, m_contentProxyModel, &QtTsProxy::setFilterText);
}

QtTsView::~QtTsView() = default;

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

void QtTsView::cancelFind()
{
    find("", Core::TextDocument::NoFindFlags); // Reset delegate highlighting
    m_tableView->viewport()->update();
    m_tableView->clearSelection();
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
    m_tableView->horizontalHeader()->setSectionResizeMode(QtTsModelView::TsColumn::Source, QHeaderView::Interactive);
    m_tableView->horizontalHeader()->setSectionResizeMode(QtTsModelView::TsColumn::Translation,
                                                          QHeaderView::Interactive);
    m_tableView->horizontalHeader()->setSectionResizeMode(QtTsModelView::TsColumn::Context, QHeaderView::Interactive);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
}

void QtTsView::find(const QString &searchText, int options)
{
    m_findAdapter->find(searchText, options);
}

} // namespace Gui
