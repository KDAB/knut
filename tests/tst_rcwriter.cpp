/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "rccore/rcfile.h"

#include <QBuffer>
#include <QFile>
#include <QSet>
#include <QTest>
#include <QUiLoader>

using namespace RcCore;

class TestRcwriter : public QObject
{
    Q_OBJECT

private slots:
    void testQrc()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/mainWindow/MainWindow.rc");
        auto data = rcFile.data.value("LANG_ENGLISH;SUBLANG_ENGLISH_US");

        // Default settings test
        {
            QBuffer buffer;
            if (buffer.open(QIODevice::WriteOnly)) {
                auto assets = convertAssets(data, Asset::NoFlags);
                writeAssetsToQrc(assets, &buffer, Test::testDataPath() + "/rcfiles/mainWindow/MainWindow.qrc");
                buffer.close();
            }
            if (buffer.open(QIODevice::ReadOnly)) {
                QFile file(Test::testDataPath() + "/tst_rcwriter/qrc_expected_default.qrc");
                file.open(QIODevice::ReadOnly);
                QCOMPARE(buffer.readAll(), file.readAll());
            }
        }

        // Don't add if asset does not exist, split toolbars
        {
            QBuffer buffer;
            if (buffer.open(QIODevice::WriteOnly)) {
                auto assets = convertAssets(data);
                writeAssetsToQrc(assets, &buffer, Test::testDataPath() + "/rcfiles/mainWindow/MainWindow.qrc");
                buffer.close();
            }
            if (buffer.open(QIODevice::ReadOnly)) {
                QFile file(Test::testDataPath() + "/tst_rcwriter/qrc_expected_noalias_onlyexist.qrc");
                file.open(QIODevice::ReadOnly);
                QCOMPARE(buffer.readAll(), file.readAll());
            }
        }
    }

    void testConvertDialog()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/luaDebugger/LuaDebugger.rc");
        auto data = rcFile.data.value("LANG_GERMAN;SUBLANG_GERMAN");
        auto result = convertDialog(data, data.dialogs.at(1), RcCore::Widget::AllFlags);

        QCOMPARE(result.id, "IDD_GOTO");
        QCOMPARE(result.geometry, QRect(0, 0, 273, 95));
        QCOMPARE(result.className, "QDialog");
        QCOMPARE(result.properties["windowTitle"].toString(), "Go To Line");

        QCOMPARE(result.children.size(), 4);
        auto item = result.children.at(2);
        QCOMPARE(item.className, "QPushButton");
        QCOMPARE(item.properties.value("text").toString(), "OK");
        QCOMPARE(item.geometry, QRect(96, 59, 75, 24));
        item = result.children.at(0);
        QCOMPARE(item.properties.value("text").toString(), "Line Number:");
        item = result.children.last();
        QCOMPARE(item.className, "QLineEdit");
    }

    void testWriteDialog()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/cryEdit/CryEdit.rc");
        auto data = rcFile.data.value("LANG_ENGLISH;SUBLANG_ENGLISH_US");

        QUiLoader loader;
        QSet<QString> dialogIds = {
            QLatin1String("IDD_ABCCOMPILE"),           QLatin1String("IDD_CHARACTER_EDITOR_AUTO_EVENTS"),
            QLatin1String("IDD_CHARPANEL_ANIMATION"),  QLatin1String("IDD_LIGHTING"),
            QLatin1String("IDD_PANEL_DISPLAY_STEREO"), QLatin1String("IDD_PANEL_TERRAIN_MODIFY"),
        };

        for (int index = 0; index < data.dialogs.size(); ++index) {
            Widget dialog = convertDialog(data, data.dialogs.value(index), RcCore::Widget::AllFlags);

            QBuffer buffer;
            if (buffer.open(QIODevice::WriteOnly)) {
                writeDialogToUi(dialog, &buffer);
                buffer.close();
            }
            if (buffer.open(QIODevice::ReadOnly)) {
                if (dialogIds.contains(dialog.id)) {
                    QFile file(Test::testDataPath() + QStringLiteral("/tst_rcwriter/%1.ui").arg(dialog.id));
                    // Uncomment to regenerate the expected files
                    // {
                    //     file.open(QIODevice::WriteOnly);
                    //     file.write(buffer.data());
                    //     file.close();
                    // }
                    file.open(QIODevice::ReadOnly);
                    QCOMPARE(buffer.readAll(), file.readAll());
                } else {
                    QWidget *widget = loader.load(&buffer);
                    QVERIFY2(loader.errorString().isEmpty(), dialog.id.toLatin1());
                    widget->deleteLater();
                }
            }
        }
    }

    void testConvertAction()
    {
        RcFile rcFile = parse(Test::testDataPath() + "/rcfiles/cryEdit/CryEdit.rc");
        auto data = rcFile.data.value("LANG_ENGLISH;SUBLANG_ENGLISH_US");

        // MainFrame menu and shortcuts and toolbar
        auto result = convertActions(data);

        QCOMPARE(result.size(), 690);
        auto action = result.at(1);
        QCOMPARE(action.id, "ID_FILE_OPEN_LEVEL");
        QCOMPARE(action.title, "Open...");
        QCOMPARE(action.toolTip, "Open");
        QCOMPARE(action.statusTip, "Open an existing level");
        QCOMPARE(action.shortcuts.size(), 1);
        QCOMPARE(action.shortcuts.first().event, "Ctrl+O");

        action = result.value(401);
        QCOMPARE(action.id, "ID_VIEW_ERRORREPORT");
        QCOMPARE(action.title, "&Error Report");
        QCOMPARE(action.shortcuts.size(), 0);
        QVERIFY(action.checked);
    }
};

QTEST_MAIN(TestRcwriter)

#include "tst_rcwriter.moc"
