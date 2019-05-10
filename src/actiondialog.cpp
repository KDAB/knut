#include "actiondialog.h"
#include "ui_actiondialog.h"

#include "data.h"
#include "global.h"
#include "jsrunner.h"
#include "overviewmodel.h"

#include <QHash>
#include <QSortFilterProxyModel>

class ActionFilterModel : public QSortFilterProxyModel
{
public:
    explicit ActionFilterModel(QObject *parent = nullptr)
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
            const int type = index.data(OverviewModel::TypeRole).toInt();
            const int row = index.data(OverviewModel::IndexRole).toInt();
            bool checked = m_checkStates.value({type, row}, false);
            return checked ? Qt::Checked : Qt::Unchecked;
        }
        return QSortFilterProxyModel::data(index, role);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if (role == Qt::CheckStateRole) {
            const int type = index.data(OverviewModel::TypeRole).toInt();
            const int row = index.data(OverviewModel::IndexRole).toInt();
            m_checkStates[{type, row}] = value.toBool();
            emit dataChanged(index, index, {Qt::CheckStateRole});
            return true;
        }
        return QSortFilterProxyModel::setData(index, value, role);
    }

    QList<QPair<int, int>> selectedData() const { return m_checkStates.keys(true); }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        const auto index = sourceModel()->index(source_row, 0, source_parent);
        const int data = index.data(OverviewModel::TypeRole).toInt();
        return (data == Knut::MenuData || data == Knut::AcceleratorData);
    }

private:
    QHash<QPair<int, int>, bool> m_checkStates;
};

ActionDialog::ActionDialog(Data *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ActionDialog)
    , m_data(data)
{
    ui->setupUi(this);

    ui->fileSelector->setFilter("*.js");

    auto model = new OverviewModel(this);
    model->setResourceData(m_data);
    m_filterModel = new ActionFilterModel(this);
    m_filterModel->setSourceModel(model);
    ui->treeView->setModel(m_filterModel);
    ui->treeView->expandAll();

    connect(ui->runButton, &QPushButton::clicked, this, &ActionDialog::run);
}

ActionDialog::~ActionDialog()
{
    delete ui;
}

void createActionForMenu(Data *file, QVariantList &actions, QHash<QString, int> &actionIdMap,
                         const Data::MenuItem &menu)
{
    if (menu.children.size()) {
        for (const auto &child : menu.children)
            createActionForMenu(file, actions, actionIdMap, child);
    } else if (!menu.id.isEmpty()) {
        Action action;
        action.id = menu.id;
        action.title = menu.text;
        action.checkable = menu.flags & Data::MenuItem::Checked;
        if (!menu.shortcut.isEmpty())
            action.shortcuts.push_back(menu.shortcut);
        const auto &text = file->strings.value(menu.id);
        if (!text.text.isEmpty()) {
            const auto tips = text.text.split('\n');
            action.statusTip = tips.first();
            if (tips.size() > 1)
                action.toolTip = tips.value(1);
        }
        actionIdMap[menu.id] = actions.size();
        actions.push_back(QVariant::fromValue(action));
    }
}

void ActionDialog::run()
{
    if (ui->fileSelector->fileName().isEmpty())
        return;

    auto dataList = m_filterModel->selectedData();
    QVariantList actions;
    QHash<QString, int> actionIdMap;

    for (const auto &data : dataList) {
        if (data.first == Knut::MenuData) {
            createActionForMenu(m_data, actions, actionIdMap, m_data->menus.value(data.second));
        } else {
        }
    }

    JsRunner runner(this);
    runner.setContextProperty("actions", actions);
    QString value = runner.runJavaScript(ui->fileSelector->fileName()).toString();
    ui->resultText->setText(value);
}
