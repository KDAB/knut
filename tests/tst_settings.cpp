/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "common/test_utils.h"
#include "core/project_p.h"
#include "core/settings.h"

#include <QSignalSpy>
#include <QTest>

///////////////////////////////////////////////////////////////////////////////
// Tests Data
///////////////////////////////////////////////////////////////////////////////
class SettingsFixture : public Core::Settings
{
public:
    SettingsFixture()
        : Settings(Core::Settings::Mode::Test)
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////
class TestSettings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { Q_INIT_RESOURCE(core); }

    void load()
    {
        SettingsFixture settings;

        // Default values
        QCOMPARE(settings.hasValue("/lsp"), true);
        QCOMPARE(settings.hasValue("lsp"), true);
        const auto lspServers = settings.value<std::vector<Core::LspServer>>("/lsp/servers");
        QCOMPARE(lspServers.size(), 1);
        QCOMPARE(lspServers.front().program, "clangd");
        QCOMPARE(lspServers.front().arguments.size(), 0);

        // Load settings
        settings.loadProjectSettings(Test::testDataPath() + "/tst_settings");
        const auto newServers = settings.value<std::vector<Core::LspServer>>("/lsp/servers");
        QCOMPARE(settings.hasValue("/foobar/foo"), true);
        Core::LspServer testData = {Core::Document::Type::Cpp, "notclangd", {"foo", "bar"}};
        QCOMPARE(newServers.front().program, testData.program);
        QCOMPARE(newServers.front().arguments, testData.arguments);
    }

    void getValue()
    {
        SettingsFixture settings;

        settings.loadProjectSettings(Test::testDataPath() + "/tst_settings");

        QCOMPARE(settings.value("/answer").toInt(), 42);
        QCOMPARE(settings.value("/pi").toFloat(), 3.14f);
        QCOMPARE(settings.value("/colors").toStringList(), (QStringList {"red", "green", "blue"}));

        // Test missing '/'
        QCOMPARE(settings.value("enabled").toBool(), true);
        QCOMPARE(settings.value("foo").toString(), "bar");

        // Test missing values
        QCOMPARE(settings.value("/bar", "default").toString(), "default");
        QCOMPARE(settings.value("/baz", 1).toInt(), 1);

        // Check value we can't parse
        QVERIFY(!settings.value("/numbers").isValid());
        QVERIFY(!settings.value("/foobar").isValid());
    }

    void setValue()
    {
        Test::FileTester file(Test::testDataPath() + "/tst_settings/setValue/knut.json");
        SettingsFixture settings;
        QSignalSpy settingsSaved(&settings, &Core::Settings::settingsSaved);

        settings.loadProjectSettings(Test::testDataPath() + "/tst_settings/setValue");
        QCOMPARE(settings.value("/rc/dialog_scalex").toDouble(), 1.5);

        settings.setValue("/rc/dialog_scalex", 2.0);
        QCOMPARE(settings.value("/rc/dialog_scalex").toDouble(), 2.0);

        QStringList test = {"This", "is", "a", "test."};
        settings.setValue("/thisisatest", test);
        QCOMPARE(settings.value<QStringList>("/thisisatest"), test);

        QJSValue varInt(10);
        settings.setValue("/thisisanothertest", varInt);
        QCOMPARE(settings.value("/thisisanothertest").toInt(), 10);

        QJSValue varString("test");
        settings.setValue("/thisisastringtest", varString);
        QCOMPARE(settings.value("/thisisastringtest").toString(), "test");

        QJSValue varDouble(3.5);
        settings.setValue("/thisisadoubletest", varDouble);
        QCOMPARE(settings.value("/thisisadoubletest").toDouble(), 3.5);

        QJSValue varBool(true);
        settings.setValue("/thisisabooltest", varBool);
        QCOMPARE(settings.value("/thisisabooltest").toBool(), true);

        // There's only one signal, as the save is done asynchronously
        settingsSaved.wait();
        QCOMPARE(settingsSaved.count(), 1);

        QVERIFY(file.compare());
    }
};

QTEST_MAIN(TestSettings)
#include "tst_settings.moc"
