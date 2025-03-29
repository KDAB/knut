/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "interfacesettings.h"

#include <QPlainTextEdit>
#include <QPoint>

class QLabel;
class QToolButton;

namespace Gui {

class ScriptPanel : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit ScriptPanel(QWidget *parent = nullptr);

    QWidget *toolBar() const;

    void setNewScript(const QString &script);

    bool hasScript() const;
    void runScript();
    void openScript();
    void newScriptDialog();
    void saveScript();

signals:
    void newScriptCreated();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent *mouseEvent) override;

private:
    void newScript();
    bool checkNeedToSaveScript();
    void editDialog();
    void checkEditDialogButton();
    QString createDialogFile();
    QString findMethodSignature(const QObject *object, const QString &functionName);

    void setupNewFile(const QString &scriptText, int cursorLeftMove);

    QWidget *const m_toolBar = nullptr;
    QLabel *m_scriptName = nullptr;
    QString m_fileName;
    QString m_selectedText;
    QPoint m_initialMousePos;
    QToolButton *m_editDialogButton = nullptr;
    InterfaceSettings *interfaceSettings = nullptr;
};

} // namespace Gui
