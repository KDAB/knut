#include "menudialog.h"
#include "ui_menudialog.h"

#include "data.h"
#include "global.h"
#include "jsrunner.h"
#include "overviewmodel.h"

#include <QHash>
#include <QSortFilterProxyModel>

class MenuFilterModel : public QSortFilterProxyModel
{
public:
    explicit MenuFilterModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        setRecursiveFilteringEnabled(true);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (index.parent().isValid())
            return QSortFilterProxyModel::flags(index) | Qt::ItemIsUserCheckable;
        else
            return QSortFilterProxyModel::flags(index);
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (role == Qt::CheckStateRole && index.parent().isValid()) {
            const int row = index.data(OverviewModel::IndexRole).toInt();
            const bool checked = row == m_checkedIndex;
            return checked ? Qt::Checked : Qt::Unchecked;
        }
        return QSortFilterProxyModel::data(index, role);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if (role == Qt::CheckStateRole) {
            const bool checked = value.toBool();
            const int row = index.data(OverviewModel::IndexRole).toInt();
            if (checked && m_checkedIndex != -1) {
                setData(index.sibling(m_checkedIndex, 0), false, Qt::CheckStateRole);
            }
            if (!checked && m_checkedIndex == row)
                m_checkedIndex = -1;
            if (checked)
                m_checkedIndex = row;
            emit dataChanged(index, index, {Qt::CheckStateRole});
            return true;
        }
        return QSortFilterProxyModel::setData(index, value, role);
    }

    int selectedData() const { return m_checkedIndex; }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        const auto index = sourceModel()->index(source_row, 0, source_parent);
        const int data = index.data(OverviewModel::TypeRole).toInt();
        return (data == Knut::MenuData);
    }

private:
    int m_checkedIndex = -1;
};

MenuDialog::MenuDialog(Data *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MenuDialog)
    , m_data(data)
{
    ui->setupUi(this);

    ui->fileSelector->setFilter("*.js");

    auto model = new OverviewModel(this);
    model->setResourceData(m_data);
    m_filterModel = new MenuFilterModel(this);
    m_filterModel->setSourceModel(model);
    ui->treeView->setModel(m_filterModel);
    ui->treeView->expandAll();

    connect(ui->runButton, &QPushButton::clicked, this, &MenuDialog::run);
}

MenuDialog::~MenuDialog()
{
    delete ui;
}

Menu createMenu(const Data::MenuItem &item)
{
    Menu menu;
    menu.id = item.id;
    menu.title = item.text;
    if (item.id.isEmpty() && item.text.isEmpty())
        menu.isSeparator = true;
    else if (item.children.isEmpty())
        menu.isAction = true;

    for (const auto &child : item.children)
        menu.children.push_back(QVariant::fromValue(createMenu(child)));

    return menu;
}

void MenuDialog::run()
{
    if (ui->fileSelector->fileName().isEmpty())
        return;

    const int index = m_filterModel->selectedData();
    Menu menu = createMenu(m_data->menus.value(index));

    JsRunner runner(this);
    runner.setContextProperty("menu", QVariant::fromValue(menu));
    QString value = runner.runJavaScript(ui->fileSelector->fileName()).toString();
    ui->resultText->setText(value);
}
