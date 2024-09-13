/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick
import Knut

Script {
    property var document: RcDocument {
        fileName: Dir.currentScriptPath + "/rcfiles/mainWindow/MainWindow.rc"
        Component.onCompleted: mergeAllLanguages()
    }

    function test_assets() {
        verify(document.valid)

        compare(document.assets.length, 2)
        document.convertAssets(RcDocument.RemoveUnknown | RcDocument.SplitToolBar | RcDocument.ConvertToPng);
        compare(document.assets.length, 16)
        compare(document.assets[1].id, "IDR_MAINFRAME_1")
        Message.log(document.assets[1].fileName)
        verify(document.assets[1].fileName.endsWith("res/Toolbar_1.png"))
    }

    function test_toolBar() {
        compare(document.toolBars.length, 2)
        compare(document.toolBars[0].id, "IDR_MAINFRAME")
        compare(document.toolBars[0].iconSize, Qt.size(16, 15))
        compare(document.toolBars[0].children.length, 10)
        compare(document.toolBars[0].children[1].id, "ID_FILE_OPEN")

        var toolBar = document.toolBar("IDR_MAINFRAME_256")
        compare(toolBar.id, "IDR_MAINFRAME_256")
        compare(toolBar.children.length, 10)
        verify(toolBar.children[3].isSeparator)
        compare(toolBar.children[4].id, "ID_EDIT_CUT")
    }

    function test_dialog() {
        var dialog = document.dialog("IDD_ABOUTBOX", RcDocument.UpdateGeometry | RcDocument.UpdateHierachy, 2, 2)
        compare(dialog.geometry.width, 340)
        compare(dialog.geometry.height, 124)
    }

    function test_menu() {
        var menu = document.menu("IDR_MAINFRAME")
        compare(menu.id, "IDR_MAINFRAME")
        compare(menu.children.length, 4)

        var editMenu = menu.children[1]
        var separator = editMenu.children[1]
        compare(separator.isSeparator, true)
        compare(separator.isAction, false)

        var viewMenu = menu.children[2]
        compare(viewMenu.text, "&View")
        compare(viewMenu.isAction, false)
        compare(viewMenu.isTopLevel, true)
        var subAction = viewMenu.children[1]
        compare(subAction.text, "&Status Bar")
        compare(subAction.isAction, true)
        compare(subAction.id, "ID_VIEW_STATUS_BAR")
    }

    function test_actions() {
        var actions = document.actionsFromMenu("IDR_MAINFRAME")
        compare(actions.length, 13)

        var actionFileNew = actions[0]
        compare(actionFileNew.id, "ID_FILE_NEW");
        compare(actionFileNew.title, "&New");
        compare(actionFileNew.toolTip, "New");
        compare(actionFileNew.statusTip, "Create a new document");
        compare(actionFileNew.shortcuts.length, 1);
        compare(actionFileNew.shortcuts[0].event, "Ctrl+N");
        Message.log(actionFileNew.iconPath)
        verify(actionFileNew.iconPath.endsWith("res/Toolbar_0.png"));

        var actionEditPaste = actions[9]
        compare(actionEditPaste.id, "ID_EDIT_PASTE");
        compare(actionEditPaste.title, "&Paste");
        compare(actionEditPaste.shortcuts.length, 2);
        compare(actionEditPaste.shortcuts[0].event, "Ctrl+V");
        compare(actionEditPaste.shortcuts[1].event, "Shift+Ins");
        Message.log(actionEditPaste.iconPath)
        verify(actionEditPaste.iconPath.endsWith("res/Toolbar_5.png"));

        var actionPrevPane = actions[12]
        compare(actionPrevPane.id, "ID_APP_ABOUT");
        compare(actionPrevPane.toolTip, "About");
        compare(actionPrevPane.statusTip, "Display program information, version number and copyright");
        compare(actionPrevPane.shortcuts.length, 0);
    }

    property var rcDocument: RcDocument {
        fileName: Dir.currentScriptPath + "/rcfiles/ribbon/RibbonApplication.rc"
        Component.onCompleted: mergeAllLanguages()
    }
    function test_ribbon() {
        verify(rcDocument.valid)

        let ribbon = rcDocument.ribbon("IDR_RIBBON");
        compare(ribbon.menu.recentFilesText, "Recent Documents");
        compare(ribbon.menu.text, "File");
        compare(ribbon.menu.elements.length, 9);
        let newButton = ribbon.menu.elements[1];
        compare(newButton.type, "Button");
        compare(newButton.id, "ID_FILE_NEW");

        compare(ribbon.categories.length, 2);
        let homeCategory = ribbon.categories[0];
        compare(homeCategory.text, "Home");

        compare(homeCategory.panels.length, 5);
        let findPanel = homeCategory.panels[3];
        compare(findPanel.text, "Find/Replace");
        compare(findPanel.keys, "F");
        compare(findPanel.elements.length, 4);
        let separator = findPanel.elements[1];
        verify(separator.isSeparator);

        compare(ribbon.contexts.length, 1);
    }
}
