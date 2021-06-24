#include "rctouidialog.h"
#include "ui_rctouidialog.h"

#include "core/rcdocument.h"
#include "core/settings.h"

#include <QDir>
#include <QMessageBox>

namespace Gui {

RcToUiDialog::RcToUiDialog(Core::RcDocument *document, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RcToUiDialog)
    , m_document(document)
{
    Q_ASSERT(m_document);
    ui->setupUi(this);

    ui->dirSelector->setMode(FileSelector::Mode::OpenDirectory);

    auto flags = DEFAULT_VALUE(Core::RcDocument::ConversionFlags, RcDialogFlags);
    ui->updateGeometry->setChecked(flags & Core::RcDocument::UpdateGeometry);
    ui->updateHierarchy->setChecked(flags & Core::RcDocument::UpdateHierarchy);
    ui->idForPixmap->setChecked(flags & Core::RcDocument::UseIdForPixmap);

    const double scaleX = DEFAULT_VALUE(double, RcDialogScaleX);
    ui->scaleX->setValue(scaleX);
    const double scaleY = DEFAULT_VALUE(double, RcDialogScaleY);
    ui->scaleY->setValue(scaleY);

    const QStringList dialogIds = m_document->dialogIds();
    for (const auto &id : dialogIds) {
        auto item = new QListWidgetItem(id, ui->idList);
        item->setCheckState(Qt::Checked);
    }
}

RcToUiDialog::~RcToUiDialog()
{
    delete ui;
}

void RcToUiDialog::accept()
{
    Core::RcDocument::ConversionFlags flags = Core::RcDocument::NoFlags;
    if (ui->updateGeometry->isChecked())
        flags |= Core::RcDocument::UpdateGeometry;
    if (ui->updateHierarchy->isChecked())
        flags |= Core::RcDocument::UpdateHierarchy;
    if (ui->idForPixmap->isChecked())
        flags |= Core::RcDocument::UseIdForPixmap;
    const double scaleX = ui->scaleX->value();
    const double scaleY = ui->scaleY->value();

    const QString path = ui->dirSelector->fileName();
    QDir dir(path);
    if (!dir.exists()) {
        QMessageBox::warning(nullptr, tr("Error"), tr("Unable to open directory %1.").arg(path));
        return;
    }

    const int numberOfDialogs = ui->idList->count();
    for (int i = 0; i < numberOfDialogs; ++i) {
        QListWidgetItem *item = ui->idList->item(i);
        if (item->checkState() == Qt::Checked) {
            const auto &dialog = m_document->dialog(item->text(), flags, scaleX, scaleY);
            const QString fileName = path + '/' + dialog.id + ".ui";
            if (!m_document->writeDialogToUi(dialog, fileName)) {
                QMessageBox::warning(nullptr, tr("Error"), tr("Unable to write ui file %1.").arg(fileName));
                return;
            }
        }
    }
    QDialog::accept();
}

} // namespace Gui
