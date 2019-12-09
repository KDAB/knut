#include "uidialog.h"
#include "ui_uidialog.h"

#include "converter.h"
#include "data.h"
#include "overviewfiltermodel.h"
#include "overviewmodel.h"
#include "writer.h"

#include <QFile>
#include <QSettings>

namespace {
constexpr char UiOutputDirKey[] = "uiOutputDir";
}

UiDialog::UiDialog(Data *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UiDialog)
    , m_data(data)
{
    ui->setupUi(this);
    ui->fileSelector->setMode(FileSelector::Mode::OpenDirectory);

    auto model = new OverviewModel(this);
    model->setResourceData(m_data);
    m_filterModel = new OverviewFilterModel(this);
    m_filterModel->setDataType({Knut::DialogData});
    m_filterModel->setSourceModel(model);
    ui->treeView->setModel(m_filterModel);
    ui->treeView->expandAll();

    QSettings settings;
    const QString recentDir = settings.value(QLatin1String(UiOutputDirKey)).toString();
    ui->fileSelector->setFileName(recentDir);
}

UiDialog::~UiDialog()
{
    QSettings settings;
    settings.setValue(QLatin1String(UiOutputDirKey), ui->fileSelector->fileName());
    delete ui;
}

void UiDialog::accept()
{
    if (ui->fileSelector->fileName().isEmpty())
        return QDialog::reject();

    const QVariantList dialogs = Converter::convertDialogs(m_data, m_filterModel->selectedData());

    for (const auto &value : dialogs) {
        const auto dialog = value.value<Converter::Widget>();
        QFile file(QStringLiteral("%1/%2.ui").arg(ui->fileSelector->fileName()).arg(dialog.id));

        if (file.open(QIODevice::WriteOnly))
            Writer::writeUi(&file, dialog);
    }

    return QDialog::accept();
}
