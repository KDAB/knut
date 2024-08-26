/*
  This file is part of Knut.

SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

SPDX-License-Identifier: GPL-3.0-only

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

class QLineEdit;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

#include <QWidget>

namespace Gui {

class FindInFilesPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FindInFilesPanel(QWidget *parent = nullptr);

    QWidget *toolBar() const;

private:
    void displayResults(const QVariantList &results) const;
    void findInFiles();
    void openFileAtItem(QTreeWidgetItem *item);
    void setupToolBar();

    QWidget *const m_toolBar;
    QTreeWidget *m_resultsDisplay;
    QLineEdit *m_searchInput;
};

} // namespace Gui
