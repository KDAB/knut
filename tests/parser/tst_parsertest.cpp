#include <QtTest>

#include "parser.h"

class ParserTest : public QObject
{
    Q_OBJECT

private slots:
    void testDialog() {
        Data data = Parser::parse(QString(QLatin1String(SAMPLES) + QStringLiteral("dialog/dialog/dialog.rc")));

        QCOMPARE(data.isValid, true);

        // Includes - afxres includes are discarded
        QCOMPARE(data.includes.size(), 7);
        qDebug() << " data.includes.first().fileName" << data.includes.first().fileName;
        qDebug() << " dsss " << QLatin1String(SAMPLES) + QStringLiteral("dialog/dialog/resource.h");
        QCOMPARE(data.includes.first().fileName, QString(QLatin1String(SAMPLES) + QStringLiteral("dialog/dialog/resource.h")));
        QCOMPARE(data.resourceMap.value(100), QStringLiteral("IDD_ABOUTBOX"));
        QCOMPARE(data.resourceMap.value(102), QStringLiteral("IDD_DIALOG_DIALOG"));

        QCOMPARE(data.icons.size(), 1);
        QCOMPARE(data.icons.value(QLatin1String("IDR_MAINFRAME")).fileName, QLatin1String(SAMPLES) + QStringLiteral("dialog/dialog/res/dialog.ico"));

        QCOMPARE(data.strings.size(), 1);
        QCOMPARE(data.strings.value(QLatin1String("IDS_ABOUTBOX")).text, QStringLiteral("&About dialog..."));

        QCOMPARE(data.dialogData.size(), 1);
        const auto &inits = data.dialogData.first().values.value(QStringLiteral("IDC_COMBO1"));
        QCOMPARE(inits.size(), 2);
        QCOMPARE(inits.at(0), QStringLiteral("Test"));
        QCOMPARE(inits.at(1), QStringLiteral("Hello World"));
    }

    void testAccelerator() {
        Data data = Parser::parse(QString(QLatin1String(SAMPLES) + QStringLiteral("accelerators/Accelerators.rc")));

        QCOMPARE(data.isValid, true);
        QCOMPARE(data.acceleratorTables.size(), 1);
        QCOMPARE(data.acceleratorTables.first().accelerators.size(), 14);
        QCOMPARE(data.acceleratorTables.first().accelerators.at(0).shortcut, QStringLiteral("Ctrl+C"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(1).shortcut, QStringLiteral("Shift+K"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(2).shortcut, QStringLiteral("Alt+K"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(3).shortcut, QStringLiteral("B"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(4).shortcut, QStringLiteral("Shift+B"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(5).shortcut, QStringLiteral("G"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(6).shortcut, QStringLiteral("Shift+G"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(7).shortcut, QStringLiteral("F1"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(8).shortcut, QStringLiteral("Ctrl+F1"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(9).shortcut, QStringLiteral("Shift+F1"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(10).shortcut, QStringLiteral("Alt+F1"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(11).shortcut, QStringLiteral("Alt+Shift+F2"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(12).shortcut, QStringLiteral("Ctrl+Shift+F2"));
        QCOMPARE(data.acceleratorTables.first().accelerators.at(13).shortcut, QStringLiteral("Ctrl+Alt+F2"));
    }

    void testLuaDebugger() {
        Data data = Parser::parse(QString(QLatin1String(SAMPLES) + QStringLiteral("complex/luaDebugger/LuaDebugger.rc")));

        QCOMPARE(data.isValid, true);

        // Menu
        QCOMPARE(data.menus.size(), 1);
        QCOMPARE(data.menus.first().id, QStringLiteral("IDC_LUADBG"));
        QCOMPARE(data.menus.first().children.size(), 5);

        const auto &filePopup = data.menus.first().children.first();
        QCOMPARE(filePopup.text, QStringLiteral("&File"));
        QCOMPARE(filePopup.children.size(), 1);
        QCOMPARE(filePopup.children.first().text, QStringLiteral("&Reload"));

        const auto &debugPopup = data.menus.first().children.at(2);
        QCOMPARE(debugPopup.text, QStringLiteral("&Debug"));
        QCOMPARE(debugPopup.children.size(), 15);
        const auto &toggleAction = debugPopup.children.first();
        QCOMPARE(toggleAction.id, QStringLiteral("ID_DEBUG_TOGGLEBREAKPOINT"));
        QCOMPARE(toggleAction.text, QStringLiteral("&Set Breakpoint"));
        QCOMPARE(toggleAction.shortcut, QStringLiteral("F9"));
        QCOMPARE(debugPopup.children.at(1).id, QString());
        QCOMPARE(data.menus.first().children.at(4).text, QStringLiteral("&Help"));

        // ToolBars
        QCOMPARE(data.toolBars.size(), 1);
        QCOMPARE(data.toolBars.first().id, QStringLiteral("IDC_LUADBG"));
        QCOMPARE(data.toolBars.first().width, 16);
        QCOMPARE(data.toolBars.first().height, 15);

        const auto &barChildren = data.toolBars.first().children;
        QCOMPARE(barChildren.size(), 10);
        QCOMPARE(barChildren.first().id, QStringLiteral("ID_DEBUG_RUN"));
        QCOMPARE(barChildren.at(2).id, QStringLiteral("IDM_EXIT"));
        QVERIFY(barChildren.at(3).id.isNull());
        QCOMPARE(barChildren.at(9).id, QStringLiteral("IDM_ABOUT"));

        // Dialogs
        QCOMPARE(data.dialogs.size(), 3);
        const auto gotoFuncDialog = data.dialogs.first();
        QCOMPARE(gotoFuncDialog.id, QStringLiteral("IDD_GOTO_FUNC"));
        const QStringList styles = {QStringLiteral("DS_SETFONT"),  QStringLiteral("DS_MODALFRAME"),
                                    QStringLiteral("DS_FIXEDSYS"), QStringLiteral("WS_POPUP"),
                                    QStringLiteral("WS_CAPTION"),  QStringLiteral("WS_SYSMENU")};
        QCOMPARE(gotoFuncDialog.styles, styles);
        QCOMPARE(gotoFuncDialog.caption, QStringLiteral("Go To Function"));
        QCOMPARE(gotoFuncDialog.geometry, QRect(0, 0, 262, 314));
        QCOMPARE(gotoFuncDialog.controls.size(), 3);
        QCOMPARE(gotoFuncDialog.controls.at(2).id, QStringLiteral("IDC_LIST_FUNC"));
        QCOMPARE(gotoFuncDialog.controls.at(2).geometry, QRect(7, 7, 248, 280));
        const QStringList styles2 = {QStringLiteral("LBS_SORT"), QStringLiteral("LBS_NOINTEGRALHEIGHT"),
                                     QStringLiteral("!WS_VSCROLL"), QStringLiteral("WS_TABSTOP")};
        QCOMPARE(gotoFuncDialog.controls.at(2).styles, styles2);
    }

    void test2048Game() {
        Data data = Parser::parse(QString(QLatin1String(SAMPLES) + QStringLiteral("2048/2048Game/2048Game.rc")));

        QCOMPARE(data.isValid, true);
    }

    void testCryEdit() {
        Data data = Parser::parse(QString(QLatin1String(SAMPLES) + QStringLiteral("complex/cryEdit/CryEdit.rc")));

        QCOMPARE(data.isValid, true);
    }
};

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
