#include "actiondialog.h"
#include "ui_actiondialog.h"

#include "data.h"
#include "global.h"
#include "jsrunner.h"
#include "overviewmodel.h"
#include "overviewfiltermodel.h"
#include "logging.h"


ActionDialog::ActionDialog(Data *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ActionDialog)
    , m_data(data)
{
    ui->setupUi(this);

    ui->fileSelector->setFilter("*.js");

    auto model = new OverviewModel(this);
    model->setResourceData(m_data);
    m_filterModel = new OverviewFilterModel(this);
    m_filterModel->setDataType({Knut::MenuData, Knut::AcceleratorData});
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
        // We stop here in case of duplication in the menu
        if (actionIdMap.contains(menu.id)) {
            qCWarning(CONVERTER) << "Duplicate action in menu:" << menu.id;
            return;
        }
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

void createActionForAccelerator(Data *file, QVariantList &actions, QHash<QString, int> &actionIdMap,
                                const Data::AcceleratorTable &item)
{
    for (const auto &accelerator : item.accelerators) {
        if (accelerator.shortcut.isEmpty()) {
            qCWarning(CONVERTER) << "Empty shortcut:" << accelerator.id;
            continue;
        }

        const int index = actionIdMap.value(accelerator.id, -1);
        if (index != -1) {
            auto action = actions.value(index).value<Action>();
            action.shortcuts.append(accelerator.shortcut);
            actions[index] = QVariant::fromValue(action);
        } else {
            Action action;
            action.id = accelerator.id;
            const auto &text = file->strings.value(accelerator.id);
            if (!text.text.isEmpty()) {
                const auto tips = text.text.split('\n');
                action.statusTip = tips.first();
                if (tips.size() > 1)
                    action.toolTip = tips.value(1);
            }
        }
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
            createActionForAccelerator(m_data, actions, actionIdMap, m_data->acceleratorTables.value(data.second));
        }
    }

    JsRunner runner(this);
    runner.setContextProperty("actions", actions);
    QString value = runner.runJavaScript(ui->fileSelector->fileName()).toString();
    ui->resultText->setText(value);
}
