/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "core/qtuidocument.h"
#include "core/utils.h"

#include <QTest>

class TestQtUiDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        Q_INIT_RESOURCE(core);
        Q_INIT_RESOURCE(rccore);
    }

    void load()
    {
        Core::QtUiDocument document;
        document.load(Test::testDataPath() + QStringLiteral("/tst_qtuidocument/IDD_ABCCOMPILE.ui"));

        const auto widgets = document.widgets();
        QCOMPARE(widgets.count(), 21);
        auto rootWidget = widgets.first();
        QCOMPARE(rootWidget->name(), "IDD_ABCCOMPILE");
        QCOMPARE(rootWidget->className(), "QDialog");
        QCOMPARE(rootWidget->isRoot(), true);

        auto widget = document.findWidget("IDC_RADIO_YUP");
        QCOMPARE(widget->name(), "IDC_RADIO_YUP");
        QCOMPARE(widget->className(), "QRadioButton");
        QCOMPARE(widget->isRoot(), false);

        widget = document.findWidget("NOT_EXIST");
        QCOMPARE(widget, nullptr);
    }

    void save()
    {
        Test::FileTester file(Test::testDataPath() + "/tst_qtuidocument/IDD_LIGHTING.ui");
        {
            Core::QtUiDocument document;
            document.load(file.fileName());

            const auto widgets = document.widgets();
            QCOMPARE(widgets.count(), 50);

            auto root = widgets.first();
            root->setClassName("QWidget");
            root->setName("Lighting");

            auto widget = document.findWidget("IDC_LIGHTING_SUNDIR_EDIT");
            widget->setClassName("QPushButton");
            widget->setName("LightingSundirEdit");

            document.save();

            QVERIFY(file.compare());
        }
    }

    void createFiles()
    {
        {
            Core::QtUiDocument document;

            auto root = document.addWidget("QDialog", "MyDialog");
            root->addProperty("geometry", QRect(0, 0, 200, 200));
            root->addProperty("windowTitle", "My Dialog");
            root->addProperty("idString", "IDD_MYDIALOG", {{"notr", "true"}});

            auto button = document.addWidget("QPushButton", "button", root);
            button->addProperty("geometry", QRect(10, 10, 100, 50));
            button->addProperty("text", "Click me");
            button->addProperty("default", true);

            auto button2 = document.addWidget("MyPushButton", "myButton", root);
            button2->addProperty("geometry", QRect(10, 100, 100, 50));
            button2->addProperty("text", "Click me 2");

            document.addCustomWidget("MyPushButton", "QPushButton", "<MyPushButton.h>");

            document.saveAs(Test::testDataPath() + QStringLiteral("/tst_qtuidocument/MyDialog.ui"));
        }
        QVERIFY(Test::compareFiles(Test::testDataPath() + QStringLiteral("/tst_qtuidocument/MyDialog.ui"),
                                   Test::testDataPath() + QStringLiteral("/tst_qtuidocument/MyDialog.ui.expected")));
        QFile::remove(Test::testDataPath() + QStringLiteral("/tst_qtuidocument/MyDialog.ui"));

        {
            Core::QtUiDocument document;

            auto root = document.addWidget("QMainWindow", "MyMainWindow");
            root->addProperty("geometry", QRect(0, 0, 200, 200));
            root->addProperty("windowTitle", "My MainWindow");

            auto button = document.addWidget("QPushButton", "button", root);
            button->addProperty("geometry", QRect(10, 10, 100, 50));
            document.addCustomWidget("MyFrame", "QFrame", "\"MyFrame.h\"", true);

            document.saveAs(Test::testDataPath() + QStringLiteral("/tst_qtuidocument/MyMainWindow.ui"));
        }
        QVERIFY(
            Test::compareFiles(Test::testDataPath() + QStringLiteral("/tst_qtuidocument/MyMainWindow.ui"),
                               Test::testDataPath() + QStringLiteral("/tst_qtuidocument/MyMainWindow.ui.expected")));
        QFile::remove(Test::testDataPath() + QStringLiteral("/tst_qtuidocument/MyMainWindow.ui"));
    }

    void readProperty()
    {
        {
            // Test with idString property set
            Core::QtUiDocument document;
            document.load(Test::testDataPath() + QStringLiteral("/tst_qtuidocument/TutorialDlg.ui"));

            const auto widgets = document.widgets();
            auto rootWidget = widgets.first();
            QCOMPARE(rootWidget->name(), "CTutorialDlg");
            QCOMPARE(rootWidget->getProperty("idString").toString(), "IDD_UPDATEGUI_DIALOG");
            const QRect geometry(0, 0, 480, 330);
            QCOMPARE(rootWidget->getProperty("geometry").toRect(), geometry);

            auto widget = document.findWidget("btn_add");
            QCOMPARE(widget->getProperty("idString").toString(), "ID_BTN_ADD");
            QCOMPARE(widget->getProperty("text").toString(), "Click to Add");
            QCOMPARE(widget->getProperty("default").toBool(), true);
        }

        {
            // Test without idString property set
            Core::QtUiDocument document;
            document.load(Test::testDataPath() + QStringLiteral("/tst_qtuidocument/IDD_ABCCOMPILE.ui"));

            const auto widgets = document.widgets();
            auto rootWidget = widgets.first();
            QVERIFY(rootWidget->getProperty("idString").isNull());

            auto widget = document.findWidget("IDC_PLAYBACKFROMMEMORY");
            QVERIFY(widget->getProperty("idString").isNull());
        }
    }
};

QTEST_MAIN(TestQtUiDocument)
#include "tst_qtuidocument.moc"
