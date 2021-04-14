#include "settings.h"

#include <QDir>
#include <QFile>

using json = nlohmann::json;

namespace Core {

static const char settingsName[] = ".knut";

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

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    loadKnutSettings();
    loadUserSettings();
}

Settings::~Settings() { }

Settings *Settings::instance()
{
    static Settings instance;
    return &instance;
}

void Settings::loadProjectSettings(const QString &rootDir)
{
    const QString fileName = rootDir + '/' + settingsName;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        try {
            m_settings.merge_patch(json::parse(file.readAll().constData()));
            spdlog::trace("[Settings] Loading project settings {}", fileName.toStdString());
        } catch (...) {
            spdlog::error("[Settings] Error loading the project settings, in file {}", fileName.toStdString());
        }
    } else {
        spdlog::trace("[Settings] No project settings {}", fileName.toStdString());
    }
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

QVariant Settings::value(const QString &path, const QVariant &defaultValue) const
{
    try {
        auto val = m_settings.at(json::json_pointer(path.toStdString()));
        spdlog::trace("[Settings] Getting setting value {}", path.toStdString());
        if (val.is_number_integer())
            return val.get<int>();
        else if (val.is_number_float())
            return val.get<float>();
        else if (val.is_string())
            return val.get<QString>();
        else if (val.is_array()) {
            // Only support QStringList for now
            return val.get<QStringList>();
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
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        try {
            m_settings.merge_patch(json::parse(file.readAll().constData()));
            spdlog::trace("[Settings] Loading user settings {}", fileName.toStdString());
        } catch (...) {
            spdlog::error("[Settings] Error loading the user settings, in file {}", fileName.toStdString());
        }
    } else {
        spdlog::trace("[Settings] No user settings {}", fileName.toStdString());
    }
}

} // namespace Core
