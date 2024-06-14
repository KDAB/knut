/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "qtuiview.h"
#include "core/qtuidocument.h"
#include "core/rcdocument.h"

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
}

void QtUiView::setUiDocument(Core::QtUiDocument *document)
{
    Q_ASSERT(document);

    if (m_document)
        m_document->disconnect(this);

    m_document = document;
    if (m_document)
        connect(m_document, &Core::QtUiDocument::fileUpdated, this, &QtUiView::updateView);

    updateView();
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

    qDebug() << m_previewArea->subWindowList().count();
    qDebug() << m_previewWindow->geometry();
}

} // namespace Gui
