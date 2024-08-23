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
    class FindWidget;
}

class FindWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindWidget(QWidget *parent = nullptr);
    ~FindWidget() override;

    void findNext();
    void findPrevious();

    void open();

    void setReplaceVisible(bool show = true);

signals:
    void findRequested(const QString &text, int options);
    void replaceRequested(const QString &before, const QString &after, int options, bool replaceAll);
    void widgetClosed();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    int findFlags() const;
    QString findString();

    void find(int options);
    void replaceOne();
    void replaceAll();
    void replace(bool replaceAll);

    std::unique_ptr<Ui::FindWidget> ui;
    QAction *m_matchCase = nullptr;
    QAction *m_matchWord = nullptr;
    QAction *m_matchRegexp = nullptr;
    QAction *m_preserveCase = nullptr;
    QString m_defaultString;
    bool m_isDefaultSelection = false;
    bool m_firstTime = true;
};

} // namespace Gui
