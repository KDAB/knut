#include "contenttree.h"

#include "acceleratormodel.h"
#include "assetmodel.h"
#include "global.h"
#include "includemodel.h"
#include "menumodel.h"
#include "stringmodel.h"

#include <QHeaderView>

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
    delete m_model;
    m_model = nullptr;

    switch (type) {
    case Knut::MenuData:
        if (index != -1)
            m_model = new MenuModel(m_data, &(m_data->menus[index]), this);
        break;
    case Knut::IconData:
        m_model = new AssetModel(m_data->icons, this);
        break;
    case Knut::AssetData:
        m_model = new AssetModel(m_data->assets, this);
        break;
    case Knut::StringData:
        m_model = new StringModel(m_data, this);
        break;
    case Knut::IncludeData:
        m_model = new IncludeModel(m_data, this);
        break;
    case Knut::AcceleratorData:
        if (index != -1)
            m_model = new AcceleratorModel(m_data->acceleratorTables.at(index), this);
        break;
    case Knut::DialogData:
    case Knut::ToolBarData:
    case Knut::NoData:
        break;
    }

    setModel(m_model);

    if (m_model) {
        // Need to be done after setting the model
        header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        expandAll();
        connect(selectionModel(), &QItemSelectionModel::currentChanged, this,
                &ContentTree::changeCurrentItem);
    }
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
