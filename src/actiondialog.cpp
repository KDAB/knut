#include "actiondialog.h"
#include "ui_actiondialog.h"

#include "converter.h"
#include "data.h"
#include "jsrunner.h"
#include "overviewfiltermodel.h"
#include "overviewmodel.h"

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

void ActionDialog::run()
{
    if (ui->fileSelector->fileName().isEmpty())
        return;

    QVariantList actions = Converter::convertActions(m_data, m_filterModel->selectedData());

    JsRunner runner(this);
    runner.setContextProperty("actions", actions);
    auto results = runner.runJavaScript(ui->fileSelector->fileName());
    ui->resultWidget->setResult(results);
}
