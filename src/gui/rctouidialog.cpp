/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "rctouidialog.h"
#include "core/rcdocument.h"
#include "core/settings.h"
#include "ui_rctouidialog.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>

namespace Gui {

RcToUiDialog::RcToUiDialog(Core::RcDocument *document, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RcToUiDialog)
    , m_document(document)
{
    Q_ASSERT(m_document);
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName() + ' ' + QApplication::applicationVersion() + " - " + windowTitle());

    ui->dirSelector->setMode(FileSelector::Mode::OpenDirectory);

    auto m_rcFile = &(m_document->file());
    const auto languageList = m_rcFile->data.keys();

    ui->language->clear();
    ui->language->addItems(languageList);
    const int index = ui->language->findText(m_document->language());
    ui->language->setCurrentIndex(index);

    auto flags = DEFAULT_VALUE(Core::RcDocument::ConversionFlags, RcDialogFlags);
    ui->updateGeometry->setChecked(flags & Core::RcDocument::UpdateGeometry);
    ui->updateHierarchy->setChecked(flags & Core::RcDocument::UpdateHierarchy);
    ui->idForPixmap->setChecked(flags & Core::RcDocument::UseIdForPixmap);

    const auto scaleX = DEFAULT_VALUE(double, RcDialogScaleX);
    ui->scaleX->setValue(scaleX);
    const auto scaleY = DEFAULT_VALUE(double, RcDialogScaleY);
    ui->scaleY->setValue(scaleY);

    const QStringList dialogIds = m_document->dialogIds();
    for (const auto &id : dialogIds) {
        auto item = new QListWidgetItem(id, ui->idList);
        item->setCheckState(Qt::Checked);
    }
    connect(ui->dirSelector, &FileSelector::fileNameChanged, this, [this](const QString &fileName) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!fileName.trimmed().isEmpty());
    });
    connect(ui->language, &QComboBox::currentIndexChanged, this, &RcToUiDialog::languageChanged);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

RcToUiDialog::~RcToUiDialog() = default;

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

void RcToUiDialog::languageChanged()
{
    const QString language = ui->language->currentText();
    m_document->setLanguage(language);
}

} // namespace Gui
