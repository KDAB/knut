/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "scriptprogressdialog.h"
#include "ui_scriptprogressdialog.h"

#include <QPushButton>

ScriptProgressDialog::ScriptProgressDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ScriptProgressDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->buttonBox->button(QDialogButtonBox::Yes)->setText(tr("Continue"));
    ui->logsWidget->hide();
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ScriptProgressDialog::apply);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ScriptProgressDialog::abort);
}

ScriptProgressDialog::~ScriptProgressDialog() = default;

void ScriptProgressDialog::setTitle(const QString &title)
{
    ui->progressStatusLabel->setText(title);
}

void ScriptProgressDialog::setMinimum(int minimum)
{
    ui->conversionProgressBar->setMinimum(minimum);
}

void ScriptProgressDialog::setMaximum(int maximum)
{
    ui->conversionProgressBar->setMaximum(maximum);
}

void ScriptProgressDialog::setValue(int progress)
{
    ui->conversionProgressBar->setValue(progress);
}

void ScriptProgressDialog::setInteractive(bool interactive)
{
    ui->buttonBox->setVisible(interactive);
    setModal(!interactive);
    adjustSize();
}

int ScriptProgressDialog::value() const
{
    return ui->conversionProgressBar->value();
}

void ScriptProgressDialog::setReadOnly(bool readOnly)
{
    ui->buttonBox->setEnabled(!readOnly);
}

QPlainTextEdit *ScriptProgressDialog::logsWidget()
{
    return ui->logsWidget;
}
