#include <QtTest>

#include "converter.h"
#include "parser.h"

class ConverterTest : public QObject
{
    Q_OBJECT

private:
    Data getData()
    {
        return Parser::parse(
            QString(QLatin1String(SAMPLES) + QStringLiteral("2048/2048Game/2048Game.rc")));
    }

private slots:
    void testConvertActions()
    {
        auto data = getData();
        Knut::DataCollection collection = {{Knut::MenuData, 0}, {Knut::AcceleratorData, 0}};
        auto result = Converter::convertActions(&data, collection);

        QCOMPARE(result.size(), 28);
        auto action = result.first().value<Converter::Action>();
        QCOMPARE(action.id, QStringLiteral("ID_FILE_NEW"));
        QCOMPARE(action.title, QStringLiteral("&New"));
        QCOMPARE(action.toolTip, QStringLiteral("New"));
        QCOMPARE(action.statusTip, QStringLiteral("Create a new document"));
        QCOMPARE(action.shortcuts.size(), 2);
        QCOMPARE(action.shortcuts.first().value<Converter::Shortcut>().event,
                 QStringLiteral("Ctrl+N"));
        QCOMPARE(action.shortcuts.last().value<Converter::Shortcut>().event,
                 QStringLiteral("Ctrl+N"));

        action = result.value(12).value<Converter::Action>();
        QCOMPARE(action.id, QStringLiteral("ID_EDIT_PASTE"));
        QCOMPARE(action.title, QStringLiteral("&Paste"));
        QCOMPARE(action.shortcuts.size(), 3);
        QCOMPARE(action.shortcuts.first().value<Converter::Shortcut>().event,
                 QStringLiteral("Ctrl+V"));
        QCOMPARE(action.shortcuts.last().value<Converter::Shortcut>().event,
                 QStringLiteral("Shift+Ins"));
    }

    void testConvertMenu()
    {
        auto data = getData();
        Knut::DataCollection collection = {{Knut::MenuData, 0}};
        auto result = Converter::convertMenu(&data, collection);

        QCOMPARE(result.id, QStringLiteral("IDR_MAINFRAME"));
        QCOMPARE(result.children.size(), 5);

        auto fileMenu = result.children.first().value<Converter::MenuItem>();
        QCOMPARE(fileMenu.title, QStringLiteral("&File"));
        QCOMPARE(fileMenu.children.size(), 12);
        QCOMPARE(fileMenu.isAction, false);
        QCOMPARE(fileMenu.isSeparator, false);
        QCOMPARE(fileMenu.actions.size(), 9);
        auto item = fileMenu.children.first().value<Converter::MenuItem>();
        QCOMPARE(item.id, QStringLiteral("ID_FILE_NEW"));
        QCOMPARE(item.title, QStringLiteral("&New"));
        QCOMPARE(item.isAction, true);
        QCOMPARE(item.isSeparator, false);
        item = fileMenu.children.value(4).value<Converter::MenuItem>();
        QVERIFY(item.id.isEmpty());
        QCOMPARE(item.isAction, false);
        QCOMPARE(item.isSeparator, true);
        item = fileMenu.children.value(11).value<Converter::MenuItem>();
        QCOMPARE(item.id, QStringLiteral("ID_APP_EXIT"));
        QCOMPARE(item.title, QStringLiteral("E&xit"));

        auto settingsMenu = result.children.value(4).value<Converter::MenuItem>();
        QCOMPARE(settingsMenu.title, QStringLiteral("Settings"));
        QCOMPARE(settingsMenu.children.size(), 2);
        QCOMPARE(settingsMenu.isAction, false);
        QCOMPARE(settingsMenu.isSeparator, false);
        item = settingsMenu.children.last().value<Converter::MenuItem>();
        QCOMPARE(item.id, QStringLiteral("ID_SETTINGS_BACKGROUNDCOLOR"));
        QCOMPARE(item.title, QStringLiteral("Background color"));

        auto viewMenu = result.children.value(2).value<Converter::MenuItem>();
        QCOMPARE(viewMenu.title, QStringLiteral("&View"));
        item = viewMenu.children.last().value<Converter::MenuItem>();
        QCOMPARE(item.title, QStringLiteral("&Application Look"));
        QCOMPARE(item.children.size(), 7);
        QCOMPARE(item.isAction, false);
        QCOMPARE(item.isSeparator, false);
    }

    void testConvertToolbar()
    {
        auto data = getData();
        Knut::DataCollection collection = {{Knut::ToolBarData, 4}};
        auto result = Converter::convertToolbar(&data, collection);

        QCOMPARE(result.id, QStringLiteral("IDR_MENU_IMAGES"));
        QCOMPARE(result.children.size(), 9);
        auto item = result.children.first().value<Converter::ToolBarItem>();
        QCOMPARE(item.id, QStringLiteral("ID_WINDOW_MANAGER"));
        item = result.children.value(1).value<Converter::ToolBarItem>();
        QCOMPARE(item.isSeparator, true);
        item = result.children.value(3).value<Converter::ToolBarItem>();
        QCOMPARE(item.id, QStringLiteral("ID_WINDOW_TILE_HORZ"));
        item = result.children.last().value<Converter::ToolBarItem>();
        QCOMPARE(item.id, QStringLiteral("ID_SORTING_GROUPBYTYPE"));
    }

    void testConvertDialog()
    {
        auto data = getData();
        Knut::DataCollection collection = {{Knut::DialogData, 1}};
        auto result = Converter::convertDialog(&data, collection);

        QCOMPARE(result.id, QStringLiteral("IDD_ABOUTBOX"));
        QCOMPARE(result.geometry, QRect(0, 0, 255, 102));
        QCOMPARE(result.className, QStringLiteral("QDialog"));
        QCOMPARE(result.properties[QStringLiteral("windowTitle")].toString(),
                 QStringLiteral("About 2048Game"));

        collection = {{Knut::DialogData, 0}};
        result = Converter::convertDialog(&data, collection);
        QCOMPARE(result.children.size(), 6);
        auto item = result.children.at(2).value<Converter::Widget>();
        QCOMPARE(item.className, QStringLiteral("QPushButton"));
        QCOMPARE(item.properties.value(QStringLiteral("text")).toString(), QStringLiteral("OK"));
        QCOMPARE(item.geometry, QRect(96, 108, 75, 23));
        item = result.children.last().value<Converter::Widget>();
        QCOMPARE(item.className, QStringLiteral("QComboBox"));
        QStringList values = {QStringLiteral("3"), QStringLiteral("4"), QStringLiteral("5"),
                              QStringLiteral("6")};
        QCOMPARE(item.properties.value(QStringLiteral("text")).toStringList(), values);
    }
};

QTEST_APPLESS_MAIN(ConverterTest)

#include "tst_convertertest.moc"
