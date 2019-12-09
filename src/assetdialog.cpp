#include "assetdialog.h"
#include "ui_assetdialog.h"

#include "converter.h"
#include "data.h"
#include "writer.h"

#include <QFileInfo>
#include <QSettings>

namespace {
constexpr char QrcOutputFile[] = "qrcOutputFile";
}

AssetDialog::AssetDialog(Data *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AssetDialog)
    , m_data(data)
{
    ui->setupUi(this);
    ui->fileSelector->setMode(FileSelector::Mode::SaveFile);
    ui->fileSelector->setFilter(QStringLiteral("*.qrc"));

    QSettings settings;
    const QString recentFile = settings.value(QLatin1String(QrcOutputFile)).toString();
    ui->fileSelector->setFileName(recentFile);
}

AssetDialog::~AssetDialog()
{
    QSettings settings;
    settings.setValue(QLatin1String(QrcOutputFile), ui->fileSelector->fileName());
    delete ui;
}

void AssetDialog::accept()
{
    if (ui->fileSelector->fileName().isEmpty())
        return QDialog::reject();

    const QFileInfo fi(ui->fileSelector->fileName());
    const QDir fileDir = ui->relativePaths->isChecked() ? fi.absoluteDir() : QDir();
    const auto assets = Converter::convertAssets(m_data, fileDir);

    QFile file(ui->fileSelector->fileName());

    if (file.open(QIODevice::WriteOnly))
        Writer::writeQrc(&file, assets, ui->alias->isChecked());

    return QDialog::accept();
}
