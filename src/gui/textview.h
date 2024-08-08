/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/mark.h"

#include <QWidget>

class QToolButton;

namespace Core {
class TextDocument;
}
namespace Gui {

class TextView : public QWidget
{
    Q_OBJECT
public:
    explicit TextView(QWidget *parent = nullptr);

    virtual void setDocument(Core::TextDocument *document);

    void toggleMark();
    void gotoMark();
    void selectToMark();
    bool hasMark() const;

    bool eventFilter(QObject *obj, QEvent *event) override;

protected:
    Core::TextDocument *document() const;

private:
    void updateMarkRect();
    void updateQuickActionRect();
    void showQuickActionMenu();

    Core::TextDocument *m_document = nullptr;
    std::optional<Core::Mark> m_mark = {};
    QWidget *m_markRect = nullptr;
    QToolButton *m_quickActionButton = nullptr;
};

} // namespace Gui
