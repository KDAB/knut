/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "conversionprogressdialog.h"
#include "ui_conversionprogressdialog.h"

#include <QStyle>

ConversionProgressDialog::ConversionProgressDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConversionProgressDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    ui->questionIconLabel->setPixmap(icon.pixmap(32, 32));

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ConversionProgressDialog::apply);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ConversionProgressDialog::abort);
}

ConversionProgressDialog::~ConversionProgressDialog() = default;

void ConversionProgressDialog::setLabelText(const QString &label)
{
    ui->progressStatusLabel->setText(label);
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

void ConversionProgressDialog::setMessage(const QString &message)
{
    ui->continueProgressLabel->setText(message);
}

int ConversionProgressDialog::value() const
{
    return ui->conversionProgressBar->value();
}

void ConversionProgressDialog::setBusyMode(bool busy)
{
    setModal(busy);
    ui->buttonBox->setEnabled(!busy);
}
