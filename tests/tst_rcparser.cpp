/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "rccore/rcfile.h"

#include <QTest>

using namespace RcCore;

class TestRcParser : public QObject
{
    Q_OBJECT

    static inline QString en_US = "LANG_ENGLISH;SUBLANG_ENGLISH_US";
    static inline QString fr_FR = "LANG_FRENCH;SUBLANG_FRENCH";

private slots:
    void testDialog()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/dialog/dialog.rc");

        QCOMPARE(rcFile.isValid, true);

        // Includes - afxres includes are discarded
        QCOMPARE(rcFile.includes.size(), 7);
        QCOMPARE(rcFile.includes.first().fileName, QString(Test::testDataPath() + "/rcfiles/dialog/resource.h"));
        QCOMPARE(rcFile.resourceMap.value(100), "IDD_ABOUTBOX");
        QCOMPARE(rcFile.resourceMap.value(102), "IDD_DIALOG_DIALOG");

        // Check languages
        QCOMPARE(rcFile.data.size(), 2);
        const QStringList langValues {en_US, fr_FR};
        auto langs = rcFile.data.keys();
        std::ranges::sort(langs);
        QCOMPARE(langs, langValues);

        auto usData = rcFile.data.value(en_US);
        QCOMPARE(usData.icons.size(), 0);
        QCOMPARE(usData.strings.size(), 1);
        QCOMPARE(usData.strings.value(QLatin1String("IDS_ABOUTBOX")).text, "&About dialog...");

        QCOMPARE(usData.dialogDataList.size(), 1);
        const auto &inits = usData.dialogDataList.first().values.value("IDC_COMBO1");
        QCOMPARE(inits.size(), 2);
        QCOMPARE(inits.at(0), "Test");
        QCOMPARE(inits.at(1), "Hello World");

        auto frData = rcFile.data.value(fr_FR);
        QCOMPARE(frData.icons.value(0).fileName, Test::testDataPath() + "/rcfiles/dialog/res/dialog.ico");
    }

    void testAccelerator()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/accelerators/Accelerators.rc");

        QCOMPARE(rcFile.isValid, true);
        auto data = rcFile.data.value(en_US);

        QCOMPARE(data.acceleratorTables.size(), 1);
        QCOMPARE(data.acceleratorTables.first().accelerators.size(), 14);

        QStringList shortcuts = {
            "Ctrl+C",        "Shift+K",     "Alt+K",   "B",        "Shift+B", "G",
            "Shift+G",       "F1",          "Ctrl+F1", "Shift+F1", "Alt+F1",  "Alt+Shift+F2",
            "Ctrl+Shift+F2", "Ctrl+Alt+F2",
        };
        for (int i = 0; i < shortcuts.size(); ++i)
            QCOMPARE(data.acceleratorTables.first().accelerators.at(i).shortcut, shortcuts.at(i));
    }

    void testLuaDebugger()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/luaDebugger/LuaDebugger.rc");

        QCOMPARE(rcFile.isValid, true);
        auto data = rcFile.data.value("LANG_GERMAN;SUBLANG_GERMAN");

        // Menu
        QCOMPARE(data.menus.size(), 1);
        QCOMPARE(data.menus.first().id, "IDC_LUADBG");
        QCOMPARE(data.menus.first().children.size(), 5);

        const auto &filePopup = data.menus.first().children.first();
        QCOMPARE(filePopup.text, "&File");
        QCOMPARE(filePopup.children.size(), 1);
        QCOMPARE(filePopup.children.first().text, "&Reload");

        const auto &debugPopup = data.menus.first().children.at(2);
        QCOMPARE(debugPopup.text, "&Debug");
        QCOMPARE(debugPopup.children.size(), 15);
        const auto &toggleAction = debugPopup.children.first();
        QCOMPARE(toggleAction.id, "ID_DEBUG_TOGGLEBREAKPOINT");
        QCOMPARE(toggleAction.text, "&Set Breakpoint");
        QCOMPARE(toggleAction.shortcut, "F9");
        QCOMPARE(debugPopup.children.at(1).id, QString());
        QCOMPARE(data.menus.first().children.at(4).text, "&Help");

        // ToolBars
        QCOMPARE(data.toolBars.size(), 1);
        QCOMPARE(data.toolBars.first().id, "IDC_LUADBG");
        QCOMPARE(data.toolBars.first().iconSize, QSize(16, 15));

        const auto &barChildren = data.toolBars.first().children;
        QCOMPARE(barChildren.size(), 10);
        QCOMPARE(barChildren.first().id, "ID_DEBUG_RUN");
        QCOMPARE(barChildren.at(2).id, "IDM_EXIT");
        QVERIFY(barChildren.at(3).id.isNull());
        QCOMPARE(barChildren.at(9).id, "IDM_ABOUT");

        // Dialogs
        QCOMPARE(data.dialogs.size(), 3);
        const auto gotoFuncDialog = data.dialogs.first();
        QCOMPARE(gotoFuncDialog.id, "IDD_GOTO_FUNC");
        const QStringList styles = {"DS_SETFONT", "DS_MODALFRAME", "DS_FIXEDSYS", "WS_POPUP"};
        QCOMPARE(gotoFuncDialog.styles, styles);
        QCOMPARE(gotoFuncDialog.caption, "Go To Function");
        QCOMPARE(gotoFuncDialog.geometry, QRect(0, 0, 262, 314));
        QCOMPARE(gotoFuncDialog.controls.size(), 3);
        QCOMPARE(gotoFuncDialog.controls.at(2).id, "IDC_LIST_FUNC");
        QCOMPARE(gotoFuncDialog.controls.at(2).geometry, QRect(7, 7, 248, 280));
        const QStringList styles2 = {"LBS_SORT", "LBS_NOINTEGRALHEIGHT", "!WS_VSCROLL", "WS_TABSTOP"};
        QCOMPARE(gotoFuncDialog.controls.at(2).styles, styles2);
    }

    void test2048Game()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/2048Game/2048Game.rc");
        QCOMPARE(rcFile.isValid, true);
    }

    void testCryEdit()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/cryEdit/CryEdit.rc");
        QCOMPARE(rcFile.isValid, true);
    }

    void testRibbon()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/ribbon/RibbonApplication.rc");
        QCOMPARE(rcFile.isValid, true);

        auto data = rcFile.data.value(en_US);
        QCOMPARE(data.ribbons.first().id, "IDR_RIBBON");

        auto ribbon = data.ribbon("IDR_RIBBON");
        QVERIFY(const_cast<Ribbon *>(ribbon)->load());

        QCOMPARE(ribbon->menu.recentFilesText, "Recent Documents");
        QCOMPARE(ribbon->menu.text, "File");
        QCOMPARE(ribbon->menu.smallImage, "IDB_FILESMALL");
        QCOMPARE(ribbon->menu.largeImage, "IDB_FILELARGE");
        QCOMPARE(ribbon->menu.elements.size(), 9);
        const auto &newButton = ribbon->menu.elements[1];
        QCOMPARE(newButton.type, "Button");
        QCOMPARE(newButton.id, "ID_FILE_NEW");
        QCOMPARE(newButton.text, "&New");
        QCOMPARE(newButton.largeIndex, 0);
        QCOMPARE(newButton.smallIndex, 0);

        QCOMPARE(ribbon->categories.size(), 2);
        const auto &homeCategory = ribbon->categories[0];
        QCOMPARE(homeCategory.text, "Home");
        QCOMPARE(homeCategory.keys, "H");
        QCOMPARE(homeCategory.smallImage, "IDB_WRITESMALL");
        QCOMPARE(homeCategory.largeImage, "IDB_WRITELARGE");

        QCOMPARE(homeCategory.panels.size(), 5);
        const auto &findPanel = homeCategory.panels[3];
        QCOMPARE(findPanel.text, "Find/Replace");
        QCOMPARE(findPanel.keys, "F");
        QCOMPARE(findPanel.elements.size(), 4);
        const auto &findButton = findPanel.elements[0];
        QCOMPARE(findButton.id, "ID_EDIT_FIND");
        const auto &separator = findPanel.elements[1];
        QVERIFY(separator.isSeparator());

        QCOMPARE(ribbon->contexts.size(), 1);
        const auto &context = ribbon->contexts[0];
        QCOMPARE(context.id, "ID_CONTEXT2");
        QCOMPARE(context.text, "Context1");
        QCOMPARE(context.categories.size(), 1);
    }
};

QTEST_APPLESS_MAIN(TestRcParser)

#include "tst_rcparser.moc"
