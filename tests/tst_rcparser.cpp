#include "rccore/rcfile.h"

#include "common/test_utils.h"

#include <QtTest>

using namespace RcCore;

class TestRcParser : public QObject
{
    Q_OBJECT

private slots:
    void testDialog()
    {
        Test::LogSilencer ls;
        Data data = parse(Test::testDataPath() + "/rcfiles/dialog/dialog.rc");

        QCOMPARE(data.isValid, true);

        // Includes - afxres includes are discarded
        QCOMPARE(data.includes.size(), 7);
        QCOMPARE(data.includes.first().fileName, QString(Test::testDataPath() + "/rcfiles/dialog/resource.h"));
        QCOMPARE(data.resourceMap.value(100), "IDD_ABOUTBOX");
        QCOMPARE(data.resourceMap.value(102), "IDD_DIALOG_DIALOG");

        QCOMPARE(data.icons.size(), 1);
        QCOMPARE(data.icons.value(0).fileName, Test::testDataPath() + "/rcfiles/dialog/res/dialog.ico");

        QCOMPARE(data.strings.size(), 1);
        QCOMPARE(data.strings.value(QLatin1String("IDS_ABOUTBOX")).text, "&About dialog...");

        QCOMPARE(data.dialogData.size(), 1);
        const auto &inits = data.dialogData.first().values.value("IDC_COMBO1");
        QCOMPARE(inits.size(), 2);
        QCOMPARE(inits.at(0), "Test");
        QCOMPARE(inits.at(1), "Hello World");
    }

    void testAccelerator()
    {
        Test::LogSilencer ls;
        Data data = parse(Test::testDataPath() + "/rcfiles/accelerators/Accelerators.rc");

        QCOMPARE(data.isValid, true);
        QCOMPARE(data.acceleratorTables.size(), 1);
        QCOMPARE(data.acceleratorTables.first().accelerators.size(), 14);
        QCOMPARE(data.acceleratorTables.first().accelerators.at(0).shortcut, "Ctrl+C");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(1).shortcut, "Shift+K");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(2).shortcut, "Alt+K");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(3).shortcut, "B");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(4).shortcut, "Shift+B");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(5).shortcut, "G");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(6).shortcut, "Shift+G");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(7).shortcut, "F1");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(8).shortcut, "Ctrl+F1");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(9).shortcut, "Shift+F1");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(10).shortcut, "Alt+F1");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(11).shortcut, "Alt+Shift+F2");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(12).shortcut, "Ctrl+Shift+F2");
        QCOMPARE(data.acceleratorTables.first().accelerators.at(13).shortcut, "Ctrl+Alt+F2");
    }

    void testLuaDebugger()
    {
        Test::LogSilencer ls;
        Data data = parse(Test::testDataPath() + "/rcfiles/luaDebugger/LuaDebugger.rc");

        QCOMPARE(data.isValid, true);

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
        Test::LogSilencer ls;
        Data data = parse(Test::testDataPath() + "/rcfiles/2048Game/2048Game.rc");
        QCOMPARE(data.isValid, true);
    }

    void testCryEdit()
    {
        Test::LogSilencer ls;
        Data data = parse(Test::testDataPath() + "/rcfiles/cryEdit/CryEdit.rc");
        QCOMPARE(data.isValid, true);
    }
};

QTEST_APPLESS_MAIN(TestRcParser)

#include "tst_rcparser.moc"
