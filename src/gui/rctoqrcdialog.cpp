#include "rctoqrcdialog.h"
#include "ui_rctoqrcdialog.h"

#include "core/rcdocument.h"
#include "core/settings.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

namespace Gui {

RcToQrcDialog::RcToQrcDialog(Core::RcDocument *document, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RcToQrcDialog)
    , m_document(document)
{
    Q_ASSERT(m_document);
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName() + ' ' + QApplication::applicationVersion() + " - " + windowTitle());

    auto assetFlags = DEFAULT_VALUE(Core::RcDocument::ConversionFlags, RcAssetFlags);
    ui->removeUnknown->setChecked(assetFlags & Core::RcDocument::RemoveUnknown);
    ui->convertPng->setChecked(assetFlags & Core::RcDocument::ConvertToPng);
    ui->splitToolbars->setChecked(assetFlags & Core::RcDocument::SplitToolBar);

    auto colorFlags = DEFAULT_VALUE(Core::RcDocument::ConversionFlags, RcAssetColors);
    ui->gray->setChecked(colorFlags & Core::RcDocument::Gray);
    ui->magenta->setChecked(colorFlags & Core::RcDocument::Magenta);
    ui->bottomLeft->setChecked(colorFlags & Core::RcDocument::BottomLeftPixel);

    QString qrcFileName = m_document->fileName();
    QFileInfo fi(qrcFileName);
    qrcFileName.chop(fi.suffix().length());
    qrcFileName += "qrc";
    ui->fileSelector->setFileName(qrcFileName);

    connect(ui->splitToolbars, &QCheckBox::toggled, this, &RcToQrcDialog::updateColorBox);
    connect(ui->convertPng, &QCheckBox::toggled, this, &RcToQrcDialog::updateColorBox);
}

RcToQrcDialog::~RcToQrcDialog()
{
    delete ui;
}

void RcToQrcDialog::accept()
{
    Core::RcDocument::ConversionFlags flags = Core::RcDocument::NoFlags;
    if (ui->removeUnknown->isChecked())
        flags |= Core::RcDocument::RemoveUnknown;
    if (ui->convertPng->isChecked())
        flags |= Core::RcDocument::ConvertToPng;
    if (ui->splitToolbars->isChecked())
        flags |= Core::RcDocument::SplitToolBar;

    const QString qrcFileName = ui->fileSelector->fileName();
    m_document->convertAssets(flags);
    if (!m_document->writeAssetsToQrc(qrcFileName)) {
        QMessageBox::warning(this, tr("Error"), tr("Unable to write qrc file to file %1.").arg(qrcFileName));
        return;
    }

    if (flags & (Core::RcDocument::ConvertToPng | Core::RcDocument::SplitToolBar)) {
        Core::RcDocument::ConversionFlags colors = Core::RcDocument::NoColors;
        if (ui->gray->isChecked())
            colors |= Core::RcDocument::Gray;
        if (ui->magenta->isChecked())
            colors |= Core::RcDocument::Magenta;
        if (ui->bottomLeft->isChecked())
            colors |= Core::RcDocument::BottomLeftPixel;
        m_document->writeAssetsToImage(colors);
    }

    QDialog::accept();
}

void RcToQrcDialog::updateColorBox()
{
    ui->colorBox->setEnabled(ui->convertPng->isChecked() || ui->splitToolbars->isChecked());
}

} // namespace Gui
