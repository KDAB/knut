/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QWidget>

namespace Gui {

namespace Ui {
    class ShortcutSettings;
}

class ShortcutModel;

class ShortcutSettings : public QWidget
{
    Q_OBJECT

public:
    explicit ShortcutSettings(QWidget *parent = nullptr);
    ~ShortcutSettings() override;

private:
    void updateCurrentItem();
    void resetAll();
    void resetSelected();
    void recordShortcut();

    std::unique_ptr<Ui::ShortcutSettings> ui;
    ShortcutModel *const m_shortcutModel;
};

} // namespace Gui
