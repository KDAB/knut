#include "core/settings.h"

#include "common/test_utils.h"

#include <QTest>

///////////////////////////////////////////////////////////////////////////////
// Tests Data
///////////////////////////////////////////////////////////////////////////////
// Use a test fixture to avoid using the settings singleton and loading the user settings
class SettingsFixture : public Core::Settings
{
public:
    SettingsFixture()
        : Settings(false)
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
        QCOMPARE(settings.hasValue("/lsp/cpp"), true);
        QCOMPARE(settings.hasValue("lsp/cpp"), true);
        const auto defaultServer = settings.value<Core::Settings::LspServer>("/lsp/cpp");
        QCOMPARE(defaultServer.program, "clangd");
        QCOMPARE(defaultServer.arguments.size(), 0);

        // Load settings
        settings.loadProjectSettings(Test::testDataPath() + "/settings");
        const auto newServer = settings.value<Core::Settings::LspServer>("/lsp/cpp");
        QCOMPARE(settings.hasValue("/foobar/foo"), true);
        Core::Settings::LspServer testData = {"notclangd", {"foo", "bar"}};
        QCOMPARE(newServer.program, testData.program);
        QCOMPARE(newServer.arguments, testData.arguments);
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
};

QTEST_MAIN(TestSettings)
#include "tst_settings.moc"
