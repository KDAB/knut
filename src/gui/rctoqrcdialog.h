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
    class RcToQrcDialog;
}

class RcToQrcDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RcToQrcDialog(Core::RcDocument *document, QWidget *parent = nullptr);
    ~RcToQrcDialog() override;
    void languageChanged();

public slots:
    void accept() override;

private:
    void updateColorBox();

private:
    std::unique_ptr<Ui::RcToQrcDialog> ui;
    Core::RcDocument *const m_document;
};

} // namespace Gui
