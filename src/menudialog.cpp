#include "menudialog.h"
#include "ui_menudialog.h"

#include "data.h"
#include "global.h"
#include "jsrunner.h"
#include "overviewmodel.h"
#include "overviewfiltermodel.h"

MenuDialog::MenuDialog(Data *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MenuDialog)
    , m_data(data)
{
    ui->setupUi(this);

    ui->fileSelector->setFilter("*.js");

    auto model = new OverviewModel(this);
    model->setResourceData(m_data);
    m_filterModel = new OverviewFilterModel(this);
    m_filterModel->setExclusive(true);
    m_filterModel->setDataType({Knut::MenuData});
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

    const auto &dataList = m_filterModel->selectedData();
    Q_ASSERT(dataList.size() == 1);
    const int index = dataList.first().second;
    Menu menu = createMenu(m_data->menus.value(index));

    JsRunner runner(this);
    runner.setContextProperty("menu", QVariant::fromValue(menu));
    QString value = runner.runJavaScript(ui->fileSelector->fileName()).toString();
    ui->resultText->setText(value);
}
