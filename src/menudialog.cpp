#include "menudialog.h"
#include "ui_menudialog.h"

#include "converter.h"
#include "data.h"
#include "jsrunner.h"
#include "overviewfiltermodel.h"
#include "overviewmodel.h"

MenuDialog::MenuDialog(Data *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MenuDialog)
    , m_data(data)
{
    ui->setupUi(this);

    ui->fileSelector->setFilter(QStringLiteral("*.js"));

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

void MenuDialog::run()
{
    if (ui->fileSelector->fileName().isEmpty())
        return;

    const auto menu = Converter::convertMenus(m_data, m_filterModel->selectedData());

    JsRunner runner(this);
    runner.setContextProperty(QStringLiteral("menu"), QVariant::fromValue(menu));
    auto results = runner.runJavaScript(ui->fileSelector->fileName());
    ui->resultWidget->setResult(results);
}
