#include "widgetdialog.h"
#include "ui_widgetdialog.h"

#include "converter.h"
#include "data.h"
#include "jsrunner.h"
#include "overviewfiltermodel.h"
#include "overviewmodel.h"

#include <QBuffer>
#include <QSettings>
#include <QUiLoader>

namespace  {
constexpr char JsScriptFileKey[] = "widgetJsFile";
}
WidgetDialog::WidgetDialog(Data *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WidgetDialog)
    , m_data(data)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    ui->fileSelector->setFilter(QStringLiteral("*.js"));

    auto model = new OverviewModel(this);
    model->setResourceData(m_data);
    m_filterModel = new OverviewFilterModel(this);
    m_filterModel->setDataType({Knut::DialogData});
    m_filterModel->setSourceModel(model);
    ui->treeView->setModel(m_filterModel);
    ui->treeView->expandAll();

    connect(ui->runButton, &QPushButton::clicked, this, &WidgetDialog::run);
    connect(ui->fileSelector, &FileSelector::fileNameChanged, this,
            [this](const QString &text) { ui->runButton->setEnabled(!text.trimmed().isEmpty()); });
    ui->runButton->setEnabled(false);
    QSettings settings;
    const QString recentFile = settings.value(QLatin1String(JsScriptFileKey)).toString();
    ui->fileSelector->setFileName(recentFile);
}

WidgetDialog::~WidgetDialog()
{
    QSettings settings;
    settings.setValue(QLatin1String(JsScriptFileKey), ui->fileSelector->fileName());
    delete ui;
}

void WidgetDialog::run()
{
    if (ui->fileSelector->fileName().isEmpty())
        return;

    QVariantList dialogs = Converter::convertDialogs(m_data, m_filterModel->selectedData());

    JsRunner runner(this);
    runner.setContextProperty(QStringLiteral("widgets"), dialogs);
    auto results = runner.runJavaScript(ui->fileSelector->fileName());
    ui->resultWidget->setResult(results);
}
