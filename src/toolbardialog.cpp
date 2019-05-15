#include "toolbardialog.h"
#include "ui_toolbardialog.h"

#include "converter.h"
#include "data.h"
#include "jsrunner.h"
#include "overviewfiltermodel.h"
#include "overviewmodel.h"

ToolbarDialog::ToolbarDialog(Data *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ToolbarDialog)
    , m_data(data)
{
    ui->setupUi(this);

    ui->fileSelector->setFilter(QStringLiteral("*.js"));

    auto model = new OverviewModel(this);
    model->setResourceData(m_data);
    m_filterModel = new OverviewFilterModel(this);
    m_filterModel->setExclusive(true);
    m_filterModel->setDataType({Knut::ToolBarData});
    m_filterModel->setSourceModel(model);
    ui->treeView->setModel(m_filterModel);
    ui->treeView->expandAll();

    connect(ui->runButton, &QPushButton::clicked, this, &ToolbarDialog::run);
}

ToolbarDialog::~ToolbarDialog()
{
    delete ui;
}

void ToolbarDialog::run()
{
    if (ui->fileSelector->fileName().isEmpty())
        return;

    const auto toolbar = Converter::convertToolbar(m_data, m_filterModel->selectedData());

    JsRunner runner(this);
    runner.setContextProperty(QStringLiteral("toolbars"), QVariant::fromValue(toolbar));
    auto results = runner.runJavaScript(ui->fileSelector->fileName());
    ui->resultWidget->setResult(results);
}
