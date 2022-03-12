import QtQuick 2.12
import Script 1.0
import Script.Rc 1.0
import Script.Test 1.0

TestCase {
    name: "RcDocument"

    property var document: RcDocument {
        fileName: Dir.currentScriptPath + "/rcfiles/2048Game/2048Game.rc"
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
        var actions = document.convertActions(["IDR_MAINFRAME"], ["IDR_MAINFRAME"], ["IDR_MAINFRAME"])

        compare(actions.length, 30)

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

        var actionPrevPane = actions[29]
        compare(actionPrevPane.id, "ID_PREV_PANE");
        compare(actionPrevPane.toolTip, "Previous Pane");
        compare(actionPrevPane.statusTip, "Switch back to the previous window pane");
        compare(actionPrevPane.shortcuts.length, 1);
        compare(actionPrevPane.shortcuts[0].event, "Shift+F6");
    }
}
