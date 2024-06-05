/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QDialog>

namespace Ui {
class ConversionProgressDialog;
}

class ConversionProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConversionProgressDialog(QWidget *parent = nullptr);
    ~ConversionProgressDialog() override;

    void setTitle(const QString &title);
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setValue(int progress);

    void setInteractive(bool interactive);
    void setReadOnly(bool readOnly);

    int value() const;

signals:
    void apply();
    void abort();

private:
    std::unique_ptr<Ui::ConversionProgressDialog> ui;
};
