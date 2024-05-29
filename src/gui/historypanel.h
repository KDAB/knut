/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <QTreeView>

class QToolButton;

namespace Core {
class HistoryModel;
}

namespace Gui {

class HistoryPanel : public QTreeView
{
    Q_OBJECT
public:
    explicit HistoryPanel(QWidget *parent = nullptr);

    QWidget *toolBar() const;

    bool isRecording() const;
    void startRecording();
    void stopRecording();

signals:
    void recordingChanged(bool isRecording);
    void scriptCreated(const QString &script);

private:
    void createScriptFromSelection();

    QWidget *const m_toolBar = nullptr;
    QToolButton *const m_clearButton = nullptr;
    Core::HistoryModel *const m_model = nullptr;
    int m_startRow = -1;
};

} // namespace Gui
