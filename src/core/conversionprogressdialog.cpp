/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "conversionprogressdialog.h"
#include "ui_conversionprogressdialog.h"

#include <QPushButton>

ConversionProgressDialog::ConversionProgressDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConversionProgressDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->buttonBox->button(QDialogButtonBox::Yes)->setText(tr("Continue"));
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ConversionProgressDialog::apply);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ConversionProgressDialog::abort);
}

ConversionProgressDialog::~ConversionProgressDialog() = default;

void ConversionProgressDialog::setTitle(const QString &title)
{
    ui->progressStatusLabel->setText(title);
}

void ConversionProgressDialog::setMinimum(int minimum)
{
    ui->conversionProgressBar->setMinimum(minimum);
}

void ConversionProgressDialog::setMaximum(int maximum)
{
    ui->conversionProgressBar->setMaximum(maximum);
}

void ConversionProgressDialog::setValue(int progress)
{
    ui->conversionProgressBar->setValue(progress);
}

void ConversionProgressDialog::setInteractive(bool interactive)
{
    ui->buttonBox->setVisible(interactive);
    setModal(!interactive);
    adjustSize();
}

int ConversionProgressDialog::value() const
{
    return ui->conversionProgressBar->value();
}

void ConversionProgressDialog::setReadOnly(bool readOnly)
{
    ui->buttonBox->setEnabled(!readOnly);
}
