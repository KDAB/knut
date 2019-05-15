#include "contenttree.h"

#include "acceleratormodel.h"
#include "assetmodel.h"
#include "dialogmodel.h"
#include "global.h"
#include "includemodel.h"
#include "menumodel.h"
#include "stringmodel.h"
#include "toolbarmodel.h"

#include <QHeaderView>
#include <QTreeView>
#include <QTreeWidget>
#include <QVBoxLayout>

ContentTree::ContentTree(QWidget *parent)
    : QWidget(parent)
{
    m_propertyView = new QTreeWidget(this);
    m_propertyView->setMaximumHeight(150);
    m_contentView = new QTreeView(this);

    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);

    layout->addWidget(m_propertyView);
    layout->addWidget(m_contentView);

    m_propertyView->setVisible(false);
}

void ContentTree::setResourceData(Data *data)
{
    m_data = data;
}

void ContentTree::setData(int type, int index)
{
    delete m_model;
    m_model = nullptr;
    m_propertyView->setVisible(false);

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
    case Knut::ToolBarData:
        if (index != -1)
            m_model = new ToolBarModel(m_data->toolBars.at(index), this);
        break;
    case Knut::DialogData:
        if (index != -1) {
            m_model = new DialogModel(m_data->dialogs.at(index), this);
            updateDialogProperty(m_data->dialogs.at(index));
        }
        break;
    case Knut::NoData:
        break;
    }

    m_contentView->setModel(m_model);

    if (m_model) {
        // Need to be done after setting the model
        m_contentView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        m_contentView->expandAll();
        connect(m_contentView->selectionModel(), &QItemSelectionModel::currentChanged, this,
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

void ContentTree::updateDialogProperty(const Data::Dialog &dialog)
{
    m_propertyView->clear();
    m_propertyView->setVisible(true);
    m_propertyView->setHeaderLabels({"Property", "Value"});
    m_propertyView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    m_propertyView->addTopLevelItem(new QTreeWidgetItem({"Id", dialog.id}));
    const auto geometry = QString("(%1, %2 %3x%4)")
                              .arg(dialog.geometry.x())
                              .arg(dialog.geometry.y())
                              .arg(dialog.geometry.width())
                              .arg(dialog.geometry.height());
    m_propertyView->addTopLevelItem(new QTreeWidgetItem({"Geometry", geometry}));
    m_propertyView->addTopLevelItem(new QTreeWidgetItem({"Caption", dialog.caption}));
    m_propertyView->addTopLevelItem(new QTreeWidgetItem({"Menu", dialog.menu}));
    m_propertyView->addTopLevelItem(new QTreeWidgetItem({"Styles", dialog.styles.join(QLatin1Char(','))}));
}
