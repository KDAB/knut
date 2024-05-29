/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/codedocument.h"

#include <QDialog>

namespace Gui {

namespace Ui {
    class TransformPreviewDialog;
}

class TransformPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransformPreviewDialog(Core::CodeDocument *document, const QString &resultText, int numberReplacements,
                                    QWidget *parent = nullptr);
    ~TransformPreviewDialog() override;

private:
    Ui::TransformPreviewDialog *ui;
};

} // namespace Gui
