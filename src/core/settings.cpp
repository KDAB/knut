#include "settings.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTimer>

#include <iostream>

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
    , m_saveTimer(new QTimer(this))
{
    loadKnutSettings();
    if (addUserSettings)
        loadUserSettings();

    m_saveTimer->callOnTimeout(this, &Settings::saveProjectSettings);
    m_saveTimer->setSingleShot(true);
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
    m_projectSettingsName = rootDir + '/' + settingsName;
    QFile file(m_projectSettingsName);
    if (file.open(QIODevice::ReadOnly)) {
        try {
            m_projectSettings = json::parse(file.readAll().constData());
            m_settings.merge_patch(m_projectSettings);
            spdlog::info("Loading project settings {}", m_projectSettingsName.toStdString());
        } catch (...) {
            spdlog::error("Error loading the project settings, in file {}", m_projectSettingsName.toStdString());
        }
    } else {
        spdlog::trace("No project settings {}", m_projectSettingsName.toStdString());
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

    switch (value.type()) {
    case QVariant::Bool:
        m_settings[json::json_pointer(path.toStdString())] = value.toBool();
        m_projectSettings[json::json_pointer(path.toStdString())] = value.toBool();
        break;
    case QVariant::Int:
    case QVariant::LongLong:
        m_settings[json::json_pointer(path.toStdString())] = value.toInt();
        m_projectSettings[json::json_pointer(path.toStdString())] = value.toInt();
        break;
    case QVariant::UInt:
    case QVariant::ULongLong:
        m_settings[json::json_pointer(path.toStdString())] = value.toUInt();
        m_projectSettings[json::json_pointer(path.toStdString())] = value.toUInt();
        break;
    case QVariant::Double:
        m_settings[json::json_pointer(path.toStdString())] = value.toDouble();
        m_projectSettings[json::json_pointer(path.toStdString())] = value.toDouble();
        break;
    case QVariant::String:
        m_settings[json::json_pointer(path.toStdString())] = value.toString();
        m_projectSettings[json::json_pointer(path.toStdString())] = value.toString();
        break;
    case QVariant::StringList:
        m_settings[json::json_pointer(path.toStdString())] = value.toStringList();
        m_projectSettings[json::json_pointer(path.toStdString())] = value.toStringList();
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

void Settings::loadUserSettings()
{
    const QString fileName = QDir::homePath() + '/' + settingsName;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        try {
            m_settings.merge_patch(json::parse(file.readAll().constData()));
            spdlog::trace("Loading user settings {}", fileName.toStdString());
        } catch (...) {
            spdlog::error("Error loading the user settings, in file {}", fileName.toStdString());
        }
    } else {
        spdlog::trace("No user settings {}", fileName.toStdString());
    }
}

void Settings::saveProjectSettings()
{
    if (m_projectSettingsName.isEmpty()) {
        spdlog::error("Can't save project settings, no project loaded");
        return;
    }

    QFile file(m_projectSettingsName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        spdlog::error("Error saving the user settings, in file {}", m_projectSettingsName.toStdString());
        return;
    }

    QTextStream stream(&file);
    stream << QString::fromStdString(m_projectSettings.dump(4, ' ', false));
    emit projectSettingsSaved();
}

} // namespace Core
