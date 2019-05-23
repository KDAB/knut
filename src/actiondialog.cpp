#include "actiondialog.h"
#include "ui_actiondialog.h"

#include "converter.h"
#include "data.h"
#include "jsrunner.h"
#include "overviewfiltermodel.h"
#include "overviewmodel.h"

#include <QSettings>
namespace  {
constexpr char JsScriptFileKey[] = "actionJsFile";
}
ActionDialog::ActionDialog(Data *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ActionDialog)
    , m_data(data)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    ui->fileSelector->setFilter(QStringLiteral("*.js"));

    auto model = new OverviewModel(this);
    model->setResourceData(m_data);
    m_filterModel = new OverviewFilterModel(this);
    m_filterModel->setDataType({Knut::MenuData, Knut::AcceleratorData});
    m_filterModel->setSourceModel(model);
    ui->treeView->setModel(m_filterModel);
    ui->treeView->expandAll();

    connect(ui->runButton, &QPushButton::clicked, this, &ActionDialog::run);
    connect(ui->fileSelector, &FileSelector::fileNameChanged, this, [this](const QString &text) {
        ui->runButton->setEnabled(!text.trimmed().isEmpty());
    });
    ui->runButton->setEnabled(false);
    QSettings settings;
    const QString recentFile = settings.value(QLatin1String(JsScriptFileKey)).toString();
    ui->fileSelector->setFileName(recentFile);
}

ActionDialog::~ActionDialog()
{
    QSettings settings;
    settings.setValue(QLatin1String(JsScriptFileKey), ui->fileSelector->fileName());
    delete ui;
}

void ActionDialog::run()
{
    if (ui->fileSelector->fileName().isEmpty())
        return;

    QVariantList actions = Converter::convertActions(m_data, m_filterModel->selectedData());

    JsRunner runner(this);
    runner.setContextProperty(QStringLiteral("actions"), actions);
    auto results = runner.runJavaScript(ui->fileSelector->fileName());
    ui->resultWidget->setResult(results);
}
