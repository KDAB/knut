/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "core/qtuidocument.h"

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
};

QTEST_MAIN(TestQtUiDocument)
#include "tst_qtuidocument.moc"
