#include "core/project_p.h"
#include "core/settings.h"

#include "common/test_utils.h"

#include <QSignalSpy>
#include <QTest>

///////////////////////////////////////////////////////////////////////////////
// Tests Data
///////////////////////////////////////////////////////////////////////////////
// Use a test fixture as Settings is a singleton
class SettingsFixture : public Core::Settings
{
public:
    SettingsFixture()
        : Settings()
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
        Test::LogSilencer ls;
        SettingsFixture settings;

        // Default values
        QCOMPARE(settings.hasValue("/lsp"), true);
        QCOMPARE(settings.hasValue("lsp"), true);
        const auto lspServers = settings.value<std::vector<Core::LspServer>>("/lsp");
        QCOMPARE(lspServers.size(), 1);
        QCOMPARE(lspServers.front().program, "clangd");
        QCOMPARE(lspServers.front().arguments.size(), 0);

        // Load settings
        settings.loadProjectSettings(Test::testDataPath() + "/settings");
        const auto newServers = settings.value<std::vector<Core::LspServer>>("/lsp");
        QCOMPARE(settings.hasValue("/foobar/foo"), true);
        Core::LspServer testData = {Core::Document::Type::Cpp, "notclangd", {"foo", "bar"}};
        QCOMPARE(newServers.front().program, testData.program);
        QCOMPARE(newServers.front().arguments, testData.arguments);
    }

    void getValue()
    {
        Test::LogSilencer ls;
        SettingsFixture settings;

        settings.loadProjectSettings(Test::testDataPath() + "/settings");

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
        Test::FileTester file(Test::testDataPath() + "/settings/setValue/knut_original.json");
        Test::LogSilencer ls;
        SettingsFixture settings;
        QSignalSpy settingsSaved(&settings, &Core::Settings::settingsSaved);

        settings.loadProjectSettings(Test::testDataPath() + "/settings/setValue");
        QCOMPARE(settings.value("/rc/dialog_scalex").toDouble(), 1.5);

        settings.setValue("/rc/dialog_scalex", 2.0);
        QCOMPARE(settings.value("/rc/dialog_scalex").toDouble(), 2.0);

        QStringList test = {"This", "is", "a", "test."};
        settings.setValue("/thisisatest", test);
        QCOMPARE(settings.value<QStringList>("/thisisatest"), test);

        QVariant var(10);
        settings.setValue("/thisisanothertest", var);
        QCOMPARE(settings.value("/thisisanothertest").toInt(), 10);

        QVariant v;
        QVERIFY(!settings.setValue("/shouldnotwork", v));

        QRect r;
        QVERIFY(!settings.setValue("/shouldnotwork", QVariant(r)));

        // There's only one signal, as the save is done asynchronously
        settingsSaved.wait();
        QCOMPARE(settingsSaved.count(), 1);

        QVERIFY(file.compare());
    }
};

QTEST_MAIN(TestSettings)
#include "tst_settings.moc"
