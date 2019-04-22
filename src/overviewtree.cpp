#include "overviewtree.h"

#include "data.h"
#include "global.h"
#include "overviewmodel.h"

OverviewTree::OverviewTree(QWidget *parent)
    : QTreeView(parent)
    , m_model(new OverviewModel(this))
{
    setModel(m_model);
    connect(selectionModel(), &QItemSelectionModel::currentChanged, this,
            &OverviewTree::changeCurrentItem);
}

void OverviewTree::setResourceData(Data *data)
{
    m_model->setResourceData(data);
}

void OverviewTree::updateModel()
{
    m_model->updateModel();
    selectionModel()->clearSelection();
}

void OverviewTree::changeCurrentItem(const QModelIndex &current)
{
    if (current.isValid()) {
        const int type = current.data(OverviewModel::TypeRole).toInt();
        const int index = current.data(OverviewModel::IndexRole).toInt();
        emit dataSelected(type, index);
        emit rcLineChanged(current.data(Knut::LineRole).toInt());
    } else {
        emit dataSelected(Knut::NoData, -1);
        emit rcLineChanged(-1);
    }
}
