/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QListWidget>

class QMainWindow;

namespace Gui {

class DocumentPalette : public QListWidget
{
    Q_OBJECT
public:
    explicit DocumentPalette(QMainWindow *parent);

    void showWindow();

protected:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void fillList();
    void updateListHeight();
    void changeCurrentDocument(int index);
};

} // namespace Gui
