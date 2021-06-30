#include "uiview.h"

#include "core/uidocument.h"

#include <QAbstractTableModel>
#include <QFile>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMdiArea>
#include <QMessageBox>
#include <QSplitter>
#include <QTableView>
#include <QUiLoader>

namespace Gui {

class UiModelView : public QAbstractTableModel
{
public:
    UiModelView(Core::UiDocument *document)
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
            case 0:
                return widget->name();
            case 1:
                return widget->className();
            }
        }
        return {};
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0:
                return "Name";
            case 1:
                return "Class Name";
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
        case 0:
            widget->setName(value.toString());
            break;
        case 1:
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
    Core::UiDocument *m_document = nullptr;
};

UiView::UiView(QWidget *parent)
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

void UiView::setUiDocument(Core::UiDocument *document)
{
    Q_ASSERT(document);

    m_tableView->setModel(new UiModelView(document));
    m_tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    m_tableView->setMinimumWidth(m_tableView->horizontalHeader()->sectionSize(0)
                                 + m_tableView->horizontalHeader()->sectionSize(1) + 2 * m_tableView->frameWidth());

    QUiLoader loader;
    QFile file(document->fileName());
    if (file.open(QIODevice::ReadOnly)) {
        QWidget *widget = loader.load(&file);
        if (!widget) {
            QMessageBox::warning(this, "Knut Ui View",
                                 tr("Can't load the ui file due to some errors:\n%1").arg(loader.errorString()));
            return;
        }
        widget->setMinimumSize(widget->size());
        m_previewArea->addSubWindow(widget, Qt::CustomizeWindowHint);
    }
}

} // namespace Gui
