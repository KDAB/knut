/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qttsview.h"
#include "core/qttsdocument.h"

#include <QAbstractTableModel>
#include <QHeaderView>
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
        case 0:
            widget->setComment(value.toString());
            break;
        case 1:
            widget->setSource(value.toString());
            break;
        case 2:
            widget->setTranslation(value.toString());
            break;
        }
        return true;
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (index.column() < 3) {
            return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        }
        return QAbstractTableModel::flags(index);
    }

private:
    Core::QtTsDocument *m_document = nullptr;
};

QtTsView::QtTsView(QWidget *parent)
    : QTableView(parent)
{
    verticalHeader()->hide();
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

void QtTsView::setTsDocument(Core::QtTsDocument *document)
{
    Q_ASSERT(document);

    if (m_document)
        m_document->disconnect(this);

    m_document = document;
    if (m_document)
        connect(m_document, &Core::QtTsDocument::fileUpdated, this, &QtTsView::updateView);

    updateView();
}

void QtTsView::updateView()
{
    delete model();

    setModel(new QtTsModelView(m_document));
    horizontalHeader()->resizeSection(QtTsModelView::TsColumn::Source, 200);
    horizontalHeader()->resizeSection(QtTsModelView::TsColumn::Translation, 200);
    horizontalHeader()->resizeSection(QtTsModelView::TsColumn::Context, 200);
    horizontalHeader()->setSectionResizeMode(QtTsModelView::TsColumn::Source, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(QtTsModelView::TsColumn::Translation, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(QtTsModelView::TsColumn::Context, QHeaderView::Interactive);
    horizontalHeader()->setStretchLastSection(true);
}

}
