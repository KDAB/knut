/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "core/jsondocument.h"

#include <QTest>

class TestJsonDocument : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void load()
    {
        Core::JsonDocument document;
        document.load(Test::testDataPath() + QStringLiteral("/tst_jsondocument/knut.json"));
        QVERIFY(QFile::exists(document.fileName()));
        QVERIFY(document.fileName().endsWith("knut.json"));
    }

    void hasValue()
    {
        Core::JsonDocument document;
        document.load(Test::testDataPath() + QStringLiteral("/tst_jsondocument/knut.json"));
        QVERIFY(document.hasValue("lsp"));
        QVERIFY(document.hasValue("/lsp"));
        QVERIFY(document.hasValue("lsp/servers"));
        QVERIFY(document.hasValue("/lsp/servers"));
    }

    void value()
    {
        Core::JsonDocument document;
        document.load(Test::testDataPath() + QStringLiteral("/tst_jsondocument/knut.json"));

        // Test with prefix '/'
        QCOMPARE(document.value("/answer").toInt(), 42);
        QCOMPARE(document.value("/pi").toFloat(), 3.14f);
        QCOMPARE(document.value("/colors").toStringList(), (QStringList {"red", "green", "blue"}));

        // Test missing prefix '/'
        QCOMPARE(document.value("enabled").toBool(), true);
        QCOMPARE(document.value("foo").toString(), "bar");

        // Test missing values
        QCOMPARE(document.value("/bar", "default").toString(), "default");
        QCOMPARE(document.value("/baz", 1).toInt(), 1);

        // Check value we can't parse
        QVERIFY(!document.value("/numbers").isValid());
        QVERIFY(!document.value("/foobar").isValid());
    }

    void setValue()
    {
        Core::JsonDocument document;
        document.load(Test::testDataPath() + QStringLiteral("/tst_jsondocument/knut.json"));

        // int
        QCOMPARE(document.value("/answer").toInt(), 42);
        document.setValue("answer", 24);
        QCOMPARE(document.value("/answer").toInt(), 24);
        QVariant var(42);
        document.setValue("answer", var);
        QCOMPARE(document.value("/answer").toInt(), 42);

        // double
        document.setValue("double", 24.25);
        QCOMPARE(document.value("/double").toDouble(), 24.25);

        // bool
        document.setValue("checked", true);
        QCOMPARE(document.value("/checked"), true);
        document.setValue("/checked", false);
        QCOMPARE(document.value("checked"), false);

        // string (new paths)
        document.setValue("dev/name", "giroday");
        QCOMPARE(document.value("/dev/name").toString(), "giroday");
        document.setValue("/dev/address", "somewhere in the world");
        QCOMPARE(document.value("dev/address").toString(), "somewhere in the world");

        // string list
        QStringList numbers = {"1", "2", "3"};
        document.setValue("list", numbers);
        QCOMPARE(document.value("/list").toStringList(),
                 QStringList() << "1"
                               << "2"
                               << "3");
    }
};

QTEST_MAIN(TestJsonDocument)
#include "tst_jsondocument.moc"
