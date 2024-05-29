/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QDialog>
#include <memory>

namespace Core {
class RcDocument;
}

namespace Gui {

namespace Ui {
    class RcToUiDialog;
}

class RcToUiDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RcToUiDialog(Core::RcDocument *document, QWidget *parent = nullptr);
    ~RcToUiDialog() override;
    void languageChanged();

public slots:
    void accept() override;

private:
    std::unique_ptr<Ui::RcToUiDialog> ui;
    Core::RcDocument *const m_document;
};

} // namespace Gui
