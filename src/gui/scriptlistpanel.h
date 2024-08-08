/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QTreeView>

class QToolButton;
class QLineEdit;

namespace Gui {

class ScriptListPanel : public QTreeView
{

    Q_OBJECT
public:
    explicit ScriptListPanel(QWidget *parent = nullptr);

    QWidget *toolBar() const;

    void setModel(QAbstractItemModel *model) override;

private:
    void runScript(const QModelIndex &index);
    void updateRunButton();

    QWidget *const m_toolBar = nullptr;
    QLineEdit *m_filter = nullptr;
    QToolButton *m_runButton = nullptr;
};

} // namespace Gui
