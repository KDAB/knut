#include "settings.h"

#include "project.h"
#include "scriptmanager.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

using json = nlohmann::json;

namespace Core {

static const char SettingsName[] = "knut.json";

/*!
 * \qmltype Settings
 * \brief Singleton for accessing and editing persistent settings.
 * \instantiates Core::Settings
 * \inqmlmodule Script
 * \since 4.0
 *
 * The settings are stored in a json file, and could be:
 *
 * - bool
 * - int
 * - double
 * - string
 * - array<string>
 */

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_saveTimer(new QTimer(this))
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;

    loadKnutSettings();
    m_saveTimer->callOnTimeout(this, [this]() {
        saveSettings(m_projectPathName, m_projectSettings);
    });
    m_saveTimer->setSingleShot(true);
}

Settings::~Settings()
{
    m_instance = nullptr;
}

Settings *Settings::instance()
{
    Q_ASSERT(m_instance);
    return m_instance;
}

void Settings::loadUserSettings()
{
    m_userPathName = QDir::homePath() + '/' + SettingsName;
    auto userSettings = loadSettings(m_userPathName);
    if (userSettings)
        m_settings.merge_patch(userSettings.value());
}

void Settings::loadProjectSettings(const QString &rootDir)
{
    m_projectPathName = rootDir + '/' + SettingsName;
    auto userSettings = loadSettings(m_projectPathName);
    if (userSettings) {
        m_projectSettings = userSettings.value();
        m_settings.merge_patch(m_projectSettings);
    }
}

/*!
 * \qmlmethod bool ProjectSettings::hasValue( string path)
 * Returns true if the project settings has a settings `path`.
 */
bool Settings::hasValue(QString path) const
{
    if (!path.startsWith('/'))
        path.prepend('/');
    return m_settings.contains(json::json_pointer(path.toStdString()));
}

/*!
 * \qmlmethod variant Settings::value( string path, variant defaultValue = null)
 * Returns the value of the settings `path`, or `defaultValue` if the settings does not exist.
 */
QVariant Settings::value(QString path, const QVariant &defaultValue) const
{
    try {
        if (!path.startsWith('/'))
            path.prepend('/');
        auto val = m_settings.at(json::json_pointer(path.toStdString()));
        spdlog::trace("Getting setting value {}", path.toStdString());
        if (val.is_number_unsigned())
            return val.get<unsigned int>();
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
        spdlog::error("Can't convert setting value {}", path.toStdString());
    } catch (...) {
        spdlog::debug("Trying to access non-existing setting value {}", path.toStdString());
    }
    return defaultValue;
}

/*!
 * \qmlmethod variant Settings::setValue( string path, variant value)
 * Adds a new value `value` to the project settings at the given `path`. Returns `true` if the operation succeeded.
 */
bool Settings::setValue(QString path, const QVariant &value)
{
    if (value.isNull()) {
        spdlog::error("Can't save setting value {} - {}", path.toStdString(), value.toString().toStdString());
        return false;
    }

    if (!path.startsWith('/'))
        path.prepend('/');

    auto jsonPath = json::json_pointer(path.toStdString());
    switch (value.type()) {
    case QVariant::Bool:
        m_settings[jsonPath] = value.toBool();
        m_projectSettings[jsonPath] = value.toBool();
        break;
    case QVariant::Int:
    case QVariant::LongLong:
        m_settings[jsonPath] = value.toInt();
        m_projectSettings[jsonPath] = value.toInt();
        break;
    case QVariant::UInt:
    case QVariant::ULongLong:
        m_settings[jsonPath] = value.toUInt();
        m_projectSettings[jsonPath] = value.toUInt();
        break;
    case QVariant::Double:
        m_settings[jsonPath] = value.toDouble();
        m_projectSettings[jsonPath] = value.toDouble();
        break;
    case QVariant::String:
        m_settings[jsonPath] = value.toString();
        m_projectSettings[jsonPath] = value.toString();
        break;
    case QVariant::StringList:
        m_settings[jsonPath] = value.toStringList();
        m_projectSettings[jsonPath] = value.toStringList();
        break;
    default:
        spdlog::error("Can't save setting value {} - {}", path.toStdString(), value.toString().toStdString());
        return false;
    }
    // Asynchronous save
    m_saveTimer->start();
    return true;
}

void Settings::loadKnutSettings()
{
    QFile file(":/core/settings.json");
    if (file.open(QIODevice::ReadOnly))
        m_settings = json::parse(file.readAll().constData());
}

std::optional<nlohmann::json> Settings::loadSettings(const QString &name, bool log)
{
    if (name.isEmpty())
        return {};

    QFile file(name);
    if (file.open(QIODevice::ReadOnly)) {
        try {
            auto settings = json::parse(file.readAll().constData());
            if (log)
                spdlog::trace("Loading settings {}", name.toStdString());
            // Load script paths if any
            try {
                auto scriptPaths = settings.at(nlohmann::json::json_pointer(ScriptPaths)).get<QStringList>();
                for (const auto &path : scriptPaths)
                    ScriptManager::instance()->addDirectory(path);
            } catch (...) {
            }
            return settings;
        } catch (...) {
            if (log)
                spdlog::error("Error loading the settings in file {}", name.toStdString());
        }
    } else {
        if (log)
            spdlog::trace("No settings {}", name.toStdString());
        return nlohmann::json {};
    }
    return {};
}

void Settings::saveSettings(const QString &name, const nlohmann::json &settings)
{
    if (name.isEmpty()) {
        spdlog::error("Can't save project settings, no project loaded");
        return;
    }

    QFile file(name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        spdlog::error("Error saving settings in file {}", name.toStdString());
        return;
    }

    QTextStream stream(&file);
    stream << QString::fromStdString(settings.dump(4, ' ', false));
    if (name == m_projectPathName)
        emit projectSettingsSaved();
}

void Settings::addScriptPath(const QString &path)
{
    updateScriptPaths(path, true);
}

void Settings::removeScriptPath(const QString &path)
{
    updateScriptPaths(path, false);
}

void Settings::updateScriptPaths(const QString &path, bool add)
{
    const QString rootPath = Project::instance()->root();
    bool isUSer = rootPath.isEmpty() || !path.contains(rootPath);

    if (add)
        ScriptManager::instance()->addDirectory(path);
    else
        ScriptManager::instance()->removeDirectory(path);

    const QString name = isUSer ? m_userPathName : m_projectPathName;
    auto settingsResult = loadSettings(name, false);
    if (settingsResult) {
        auto settings = settingsResult.value();
        QStringList paths;
        try {
            paths = settings.at(nlohmann::json::json_pointer(ScriptPaths)).get<QStringList>();
        } catch (...) {
        }
        if (add)
            paths.push_back(path);
        else
            paths.removeAll(path);
        settings[nlohmann::json::json_pointer(ScriptPaths)] = paths;
        saveSettings(name, settings);
    }
}

} // namespace Core
