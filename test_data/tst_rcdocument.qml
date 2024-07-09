/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick
import Knut
import Knut.Test

TestCase {
    name: "RcDocument"

    property var document: RcDocument {
        fileName: Dir.currentScriptPath + "/rcfiles/2048Game/2048Game.rc"
        Component.onCompleted: mergeAllLanguages()
    }

    function test_assets() {
        verify(document.valid)

        compare(document.assets.length, 14)
        document.convertAssets(RcDocument.RemoveUnknown | RcDocument.SplitToolBar | RcDocument.ConvertToPng);
        compare(document.assets.length, 34)
        compare(document.assets[1].id, "IDR_MAINFRAME_1")
        verify(document.assets[1].fileName.endsWith("res/Toolbar_1.png"))
    }

    function test_toolBar() {
        compare(document.toolBars.length, 6)
        compare(document.toolBars[2].id, "IDR_SORT")
        compare(document.toolBars[2].iconSize, Qt.size(16, 15))
        compare(document.toolBars[2].children.length, 2)
        compare(document.toolBars[2].children[1].id, "ID_NEW_FOLDER")

        var toolBar = document.toolBar("IDR_MENU_IMAGES")
        compare(toolBar.id, "IDR_MENU_IMAGES")
        compare(toolBar.children.length, 9)
        verify(toolBar.children[1].isSeparator)
        compare(toolBar.children[4].id, "ID_TOOLS_MACRO")
    }

    function test_dialog() {
        var dialog = document.dialog("IDD_DIALOG1", RcDocument.UpdateGeometry | RcDocument.UpdateHierachy, 2, 2)
        compare(dialog.geometry.width, 402)
        compare(dialog.geometry.height, 174)
    }

    function test_menu() {
        var menu = document.menu("IDR_MAINFRAME")
        compare(menu.id, "IDR_MAINFRAME")
        compare(menu.children.length, 5)

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
        var subMenu = viewMenu.children[2]
        compare(subMenu.text, "&Application Look")
        compare(subMenu.isAction, false)
        compare(subMenu.isTopLevel, false)
    }

    function test_actions() {
        var actions = document.actionsFromMenu("IDR_MAINFRAME")
        compare(actions.length, 28)

        var actionFileNew = actions[0]
        compare(actionFileNew.id, "ID_FILE_NEW");
        compare(actionFileNew.title, "&New");
        compare(actionFileNew.toolTip, "New");
        compare(actionFileNew.statusTip, "Create a new document");
        compare(actionFileNew.shortcuts.length, 1);
        compare(actionFileNew.shortcuts[0].event, "Ctrl+N");
        Message.log(actionFileNew.iconPath)
        verify(actionFileNew.iconPath.endsWith("res/Toolbar_0.png"));

        var actionEditPaste = actions[12]
        compare(actionEditPaste.id, "ID_EDIT_PASTE");
        compare(actionEditPaste.title, "&Paste");
        compare(actionEditPaste.shortcuts.length, 2);
        compare(actionEditPaste.shortcuts[0].event, "Ctrl+V");
        compare(actionEditPaste.shortcuts[1].event, "Shift+Ins");
        Message.log(actionEditPaste.iconPath)
        verify(actionEditPaste.iconPath.endsWith("res/Toolbar_5.png"));

        var actionPrevPane = actions[25]
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
