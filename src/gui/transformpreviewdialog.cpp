#include "transformpreviewdialog.h"
#include "ui_transformpreviewdialog.h"

#include "guisettings.h"

#include <QPushButton>

namespace Gui {

TransformPreviewDialog::TransformPreviewDialog(Core::CodeDocument *document, const QString &resultText,
                                               int numberReplacements, QWidget *parent /* = nullptr */)
    : QDialog(parent)
    , ui(new Ui::TransformPreviewDialog)
{
    ui->setupUi(this);

    ui->replacementsLabel->setText(tr("%1 Replacements made").arg(numberReplacements));
    ui->transformedText->setPlainText(resultText);
    GuiSettings::setupDocumentTextEdit(ui->transformedText, document);

    if (auto applyButton = ui->buttonBox->button(QDialogButtonBox::Apply)) {
        connect(applyButton, &QPushButton::clicked, this, &QDialog::accept);
    }
}

TransformPreviewDialog::~TransformPreviewDialog()
{
    delete ui;
}

} // namespace Gui
