/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "scriptpanel.h"
#include "core/logger.h"
#include "core/project.h"
#include "core/scriptmanager.h"
#include "core/textdocument_p.h"
#include "guisettings.h"
#include "utils/log.h"

#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollBar>
#include <QTemporaryFile>
#include <QToolButton>

namespace Gui {

constexpr char Untitled[] = "<untitled>";

// clang-format off
constexpr char DefaultScript[] = R"(// Description of the script

function main() {

}
)";

constexpr char DefaultScriptDialog[] = R"(// Description of the script

import Knut

ScriptDialog {
    id: root

    // Function called at startup, once the dialog is setup
    function init() {

    }

    // Function called when the user click on the OK button
    onAccepted: {

    }
}
)";
// clang-format on

ScriptPanel::ScriptPanel(QWidget *parent)
    : QPlainTextEdit {parent}
    , m_toolBar(new QWidget)
{
    setWindowTitle(tr("Script Panel"));
    setObjectName("ScriptPanel");

    GuiSettings::setupFileNameTextEdit(this, "script.qml");

    // Setup titlebar
    auto layout = new QHBoxLayout(m_toolBar);
    layout->setContentsMargins({});

    auto newScriptAction = new QAction(tr("New Script"), this);
    GuiSettings::setIcon(newScriptAction, ":/gui/file-document.png");
    connect(newScriptAction, &QAction::triggered, this, &ScriptPanel::newScript);
    auto newDialogAction = new QAction(tr("New Script with Dialog"), this);
    GuiSettings::setIcon(newDialogAction, ":/gui/application.png");
    connect(newDialogAction, &QAction::triggered, this, &ScriptPanel::newScriptDialog);

    auto newButton = new QToolButton(m_toolBar);
    newButton->addActions({newScriptAction, newDialogAction});
    newButton->setDefaultAction(newScriptAction);
    newButton->setAutoRaise(true);
    layout->addWidget(newButton);
    newButton->setPopupMode(QToolButton::MenuButtonPopup);
    connect(newButton, &QToolButton::triggered, newButton, &QToolButton::setDefaultAction);

    auto createButton = [this, layout](const QString &icon, const QString &tooltip) {
        auto button = new QToolButton(m_toolBar);
        GuiSettings::setIcon(button, icon);
        button->setToolTip(tooltip);
        button->setAutoRaise(true);
        layout->addWidget(button);
        return button;
    };

    auto openButton = createButton(":/gui/folder-open.png", tr("Open"));
    connect(openButton, &QToolButton::clicked, this, &ScriptPanel::openScript);

    auto saveButton = createButton(":/gui/content-save.png", tr("Save"));
    connect(saveButton, &QToolButton::clicked, this, &ScriptPanel::saveScript);

    m_editDialogButton = createButton(":/gui/application-edit.png", tr("Edit Dialog"));
    connect(m_editDialogButton, &QToolButton::clicked, this, &ScriptPanel::editDialog);

    auto separator = new QFrame(m_toolBar);
    separator->setFrameShape(QFrame::VLine);
    layout->addWidget(separator);

    m_scriptName = new QLabel(tr(Untitled));
    layout->addWidget(m_scriptName);

    auto runButton = createButton(":/gui/play.png", tr("Run"));
    connect(runButton, &QToolButton::clicked, this, &ScriptPanel::runScript);

    layout->addSpacing(20);

    connect(this, &QPlainTextEdit::textChanged, this, &ScriptPanel::checkEditDialogButton);

    newScript();
}

QWidget *ScriptPanel::toolBar() const
{
    return m_toolBar;
}

void ScriptPanel::setNewScript(const QString &script)
{
    newScript();
    setPlainText(script);
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    emit newScriptCreated();
}

bool ScriptPanel::hasScript() const
{
    return !toPlainText().isEmpty();
}

void ScriptPanel::openScript()
{
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Open Script"), "", "Script files (*.qml *.js)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        setPlainText(file.readAll());
        m_fileName = fileName;
        QFileInfo fi(m_fileName);
        m_scriptName->setText(fi.fileName());
    } else {
        spdlog::error("Error reading script - can't open the file for reading.");
    }
}

void ScriptPanel::setupNewFile(const QString &scriptText, int cursorLeftMove)
{
    m_scriptName->setText(tr(Untitled));
    m_fileName.clear();
    clear();
    auto cursor = textCursor();
    cursor.insertText(scriptText);
    cursor.movePosition(QTextCursor::End);
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, cursorLeftMove);
    setTextCursor(cursor);
    setFocus(Qt::OtherFocusReason);
}

void ScriptPanel::newScript()
{
    setupNewFile(DefaultScript, 3);
    m_editDialogButton->hide();
}

void ScriptPanel::newScriptDialog()
{
    setupNewFile(DefaultScriptDialog, 9);
    m_editDialogButton->show();
}

void ScriptPanel::saveScript()
{
    const bool isQml = toPlainText().contains("import Knut");

    if (m_fileName.isEmpty()) {
        m_fileName = QFileDialog::getSaveFileName(this, tr("Save Script"), "",
                                                  isQml ? "Script files (*.qml)" : "Script files (*.js)");
        if (m_fileName.isEmpty())
            return;
        if (!m_fileName.endsWith(isQml ? ".qml" : ".js"))
            m_fileName.append(isQml ? ".qml" : ".js");

        QFileInfo fi(m_fileName);
        m_scriptName->setText(fi.fileName());
    }

    QFile file(m_fileName);
    if (file.open(QIODevice::WriteOnly)) {
        const QString text = toPlainText();
        file.write(text.toUtf8());
        // Create the ui file if needed
        if (isQml && text.contains("ScriptDialog"))
            createDialogFile();
    } else {
        spdlog::error("Error saving script - can't open the file for writing.");
    }
}

void ScriptPanel::editDialog()
{
    if (m_fileName.isEmpty()) {
        QMessageBox::information(this, "Script Dialog Edition",
                                 "In order to create the dialog, you need to save the script.");
        saveScript();
        if (m_fileName.isEmpty())
            return;
    }

    const QString uiFileName = createDialogFile();

    if (!QDesktopServices::openUrl(QString("file:///%1").arg(uiFileName))) {
        QMessageBox::information(this, "Script Dialog Edition",
                                 "In order to edit the dialog, you need to install the Qt Designer tool.");
        spdlog::error("Error editing dialog - Qt Designer is not found.");
    }
}

void ScriptPanel::checkEditDialogButton()
{
    if (m_editDialogButton->isVisible())
        return;

    m_editDialogButton->setVisible(toPlainText().contains("ScriptDialog"));
}

QString ScriptPanel::createDialogFile()
{
    const QFileInfo fi(m_fileName);
    QString uiFileName = fi.absolutePath() + '/' + fi.baseName() + ".ui";

    if (!QFile::exists(uiFileName) && !QFile::copy(":/gui/default-dialog.ui", uiFileName)) {
        spdlog::error("Error creating dialog - can't create the dialog file.");
        return {};
    }
    return uiFileName;
}

void ScriptPanel::runScript()
{
    Core::LoggerDisabler ld;

    if (!m_fileName.isEmpty()) {
        saveScript();
        Core::ScriptManager::instance()->runScript(m_fileName, false, false);
        return;
    }

    const QString extension = find("import Knut") ? "qml" : "js";

    QTemporaryFile file("script_XXXXXX." + extension);
    if (file.open()) {
        file.write(toPlainText().toUtf8());
        file.close();
        Core::ScriptManager::instance()->runScript(file.fileName(), false, false);
    } else {
        spdlog::error("Error running script - can't save to temporary file.");
    }
}

QString ScriptPanel::findMethodSignature(const QObject *object, const QString &functionName)
{
    const QMetaObject *metaObject = object->metaObject();
    const int methodCount = metaObject->methodCount();

    for (int i = 0; i < methodCount; ++i) {
        QMetaMethod method = metaObject->method(i);
        QString methodName = method.methodSignature();

        // Check if the method name matches
        if (methodName.startsWith(functionName)) {
            return methodName;
        }
    }

    // Function not found
    return {};
}

void ScriptPanel::mousePressEvent(QMouseEvent *mouseEvent)
{
    if (Qt::LeftButton == mouseEvent->button()) {
        // Store the initial position of the mouse press event
        m_initialMousePos = mouseEvent->pos();
    }

    QPlainTextEdit::mousePressEvent(mouseEvent);
}

void ScriptPanel::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (Qt::LeftButton == mouseEvent->button()) {
        QPoint finalMousePos = mouseEvent->pos();

        // Create a QTextCursor and set its position according to initial and final mouse positions
        QTextCursor textCursor = cursorForPosition(m_initialMousePos);
        textCursor.setPosition(cursorForPosition(finalMousePos).position(), QTextCursor::KeepAnchor);
        setTextCursor(textCursor);

        m_selectedText = textCursor.selectedText();
    }

    QPlainTextEdit::mouseReleaseEvent(mouseEvent);
}

void ScriptPanel::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Backtab)) {
        Core::indentTextInTextEdit(this, -1);
        return;
    } else if (event->key() == Qt::Key_Tab) {
        Core::indentTextInTextEdit(this, 1);
        return;
    } else if (event->key() == Qt::Key_F1) {
        interfaceSettings = new InterfaceSettings();
        QString documentationUrl;
        QString fileName;
        if (auto currentDoc = Core::Project::instance()->currentDocument()) {

            QString docType = currentDoc->metaObject()->className();
            docType.remove("Core::");
            QString signature = findMethodSignature(currentDoc, m_selectedText);
            if (!signature.isEmpty()) {
                fileName = QString("%1.html").arg(docType.toLower());
            } else {
                // API property doesn't exist, handle other cases
                // ...

                if (m_selectedText.at(0).isUpper()) {
                    fileName = QString("%1.html").arg(m_selectedText.toLower());
                } else {
                    fileName = QString("%1.html").arg(docType.toLower());
                }
                // ...
            }

            documentationUrl =
                QString("%1/API/script/%2#%3").arg(interfaceSettings->getHelpPath(), fileName, m_selectedText);
        }
        QDesktopServices::openUrl(QUrl(documentationUrl));
    }

    QPlainTextEdit::keyPressEvent(event);
}

} // namespace Gui
