#include "settings.h"

#include <QDir>
#include <QFile>

using json = nlohmann::json;

namespace Core {

static const char settingsName[] = "knut.json";

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

Settings::Settings(bool addUserSettings, QObject *parent)
    : QObject(parent)
{
    loadKnutSettings();
    if (addUserSettings)
        loadUserSettings();
}

Settings::Settings(QObject *parent)
    : Settings(true, parent)
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;
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

void Settings::loadProjectSettings(const QString &rootDir)
{
    const QString fileName = rootDir + '/' + settingsName;
    loadSettings("project", fileName);
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
        spdlog::warn("Can't convert setting value {}", path.toStdString());
    } catch (...) {
        spdlog::debug("Trying to access non-existing setting value {}", path.toStdString());
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
            spdlog::trace("Loading {} settings {}", name, fileName.toStdString());
        } catch (...) {
            spdlog::error("Error loading the {} settings, in file {}", name, fileName.toStdString());
        }
    } else {
        spdlog::trace("No {} settings {}", name, fileName.toStdString());
    }
}

}
