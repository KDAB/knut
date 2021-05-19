#include "settings.h"

#include "utils/test_utils.h"

#include <QDir>
#include <QFile>

#include <doctest/doctest.h>

using json = nlohmann::json;

namespace Core {

static const char settingsName[] = "knut.json";

/*!
 * \qmltype Settings
 * \brief Provides a singleton for accessing and editing persistent settings.
 * \instantiates Core::Settings
 * \inqmlmodule Script
 * \since Script 1.0
 * \sa Settings
 * \sa Project Settings
 *
 * Settings are stored in a json file, and can be anything.
 */

Settings::Settings(bool addUserSettings, QObject *parent)
    : QObject(parent)
{
    loadKnutSettings();
    if (addUserSettings)
        loadUserSettings();
}

Settings::~Settings() { }

Settings *Settings::instance()
{
    static Settings instance(true);
    return &instance;
}

void Settings::loadProjectSettings(const QString &rootDir)
{
    const QString fileName = rootDir + '/' + settingsName;
    loadSettings("project", fileName);
}

/*!
 * \qmlmethod bool ProjectSettings::hasValue( string path)
 * Returns true if the project settings has a settings \a key.
 */
bool Settings::hasValue(const QString &path) const
{
    return m_settings.contains(json::json_pointer(path.toStdString()));
}

/*!
 * \qmlmethod variant Settings::value( string path, variant defaultValue = null)
 * Returns the value of the settings \a path, or \a defaultValue if the settings does not exist.
 */

QVariant Settings::value(QString path, const QVariant &defaultValue) const
{
    try {
        if (!path.startsWith('/'))
            path.prepend('/');
        auto val = m_settings.at(json::json_pointer(path.toStdString()));
        spdlog::trace("[Settings] Getting setting value {}", path.toStdString());
        if (val.is_number_integer())
            return val.get<int>();
        else if (val.is_number_float())
            return val.get<float>();
        else if (val.is_boolean())
            return val.get<bool>();
        else if (val.is_string())
            return val.get<QString>();
        else if (val.is_array()) {
            // Only support QStringList for now
            if (val.size()) {
                if (val[0].is_string())
                    return val.get<QStringList>();
            } else {
                return QStringList();
            }
        }
        spdlog::warn("[Settings] Can't convert setting value {}", path.toStdString());
    } catch (...) {
        spdlog::debug("[Settings] Trying to access non-existing setting value {}", path.toStdString());
    }
    return defaultValue;
}

void Settings::loadKnutSettings()
{
    QFile file(":/core/settings.json");
    if (file.open(QIODevice::ReadOnly))
        m_settings = json::parse(file.readAll().constData());
}

void Settings::loadUserSettings()
{
    const QString fileName = QDir::homePath() + '/' + settingsName;
    loadSettings("user", fileName);
}

void Settings::loadSettings(std::string name, const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        try {
            m_settings.merge_patch(json::parse(file.readAll().constData()));
            spdlog::trace("[Settings] Loading {} settings {}", name, fileName.toStdString());
        } catch (...) {
            spdlog::error("[Settings] Error loading the {} settings, in file {}", name, fileName.toStdString());
        }
    } else {
        spdlog::trace("[Settings] No {} settings {}", name, fileName.toStdString());
    }
}

} // namespace Core

///////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////

// Use a test fixture to avoid using the settings singleton
class TestSettings : public Core::Settings
{
public:
    TestSettings()
        : Settings(false)
    {
    }
};

TEST_SUITE("core")
{
    TEST_CASE_FIXTURE(TestSettings, "settings")
    {
        SUBCASE("load settings")
        {
            Test::LogSilencer ls;

            // Default values
            CHECK_EQ(hasValue("/lsp/cpp"), true);
            const auto defaultServer = value<Core::Settings::LspServer>("/lsp/cpp");
            CHECK_EQ(defaultServer.program, "clangd");
            CHECK_EQ(defaultServer.arguments.size(), 0);

            // Load settings
            loadProjectSettings(Test::testDataPath() + "/settings");
            const auto newServer = value<Core::Settings::LspServer>("/lsp/cpp");
            CHECK_EQ(hasValue("/foobar/foo"), true);
            Core::Settings::LspServer testData = {"notclangd", {"foo", "bar"}};
            CHECK_EQ(newServer.program, testData.program);
            CHECK_EQ(newServer.arguments, testData.arguments);
        }

        SUBCASE("access settings values")
        {
            Test::LogSilencer ls;

            loadProjectSettings(Test::testDataPath() + "/settings");

            CHECK_EQ(value("/answer").toInt(), 42);
            CHECK_EQ(value("/pi").toFloat(), 3.14f);

            // Test missing '/'
            CHECK_EQ(value("enabled").toBool(), true);
            CHECK_EQ(value("foo").toString(), "bar");

            // Test missing values
            CHECK_EQ(value("/bar", "default").toString(), "default");
            CHECK_EQ(value("/colors").toStringList(), QStringList {"red", "green", "blue"});
            CHECK_FALSE(value("/numbers").isValid());
            CHECK_FALSE(value("/foobar").isValid());
        }
    }
}
