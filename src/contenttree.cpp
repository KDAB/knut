#include "contenttree.h"

#include "global.h"
#include "menumodel.h"

ContentTree::ContentTree(QWidget *parent)
    : QTreeView(parent)
{
}

void ContentTree::setResourceData(Data *data)
{
    m_data = data;
}

void ContentTree::setData(int type, int index)
{
    switch (type) {
    case Knut::MenuData:
        if (index != -1) {
            m_model = new MenuModel(m_data, &(m_data->menus[index]), this);
            setModel(m_model);
            break;
        }
        [[fallthrough]];
    case Knut::DialogData:
    case Knut::ToolBarData:
    case Knut::AcceleratorData:
    case Knut::AssetData:
    case Knut::IconData:
    case Knut::StringData:
    case Knut::IncludeData:
    case Knut::NoData:
        setModel(nullptr);
        delete m_model;
        m_model = nullptr;
        return;
    }

    // Need to be done after setting the model
    expandAll();
    connect(selectionModel(), &QItemSelectionModel::currentChanged, this,
            &ContentTree::changeCurrentItem);
}

void ContentTree::clear()
{
    setData(Knut::NoData, -1);
}

void ContentTree::changeCurrentItem(const QModelIndex &current)
{
    if (current.isValid()) {
        emit rcLineChanged(current.data(Knut::LineRole).toInt());
    } else {
        emit rcLineChanged(-1);
    }
}
