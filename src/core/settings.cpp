/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "settings.h"
#include "logger.h"
#include "rcdocument.h"
#include "scriptmanager.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <optional>

static std::optional<nlohmann::json> loadSettings(const QString &name, bool log = true)
{
    if (name.isEmpty())
        return {};

    QFile file(name);
    if (file.open(QIODevice::ReadOnly)) {
        try {
            auto settings = nlohmann::json::parse(file.readAll().constData());
            if (log)
                spdlog::debug("Settings::loadSettings {}", name);
            return settings;
        } catch (...) {
            if (log)
                spdlog::error("Settings::loadSettings {}", name);
        }
    } else {
        if (log)
            spdlog::debug("Settings::loadSettings {} - file can't be read", name);
        return "{}"_json;
    }
    return {};
}

namespace Core {

static constexpr char SettingsName[] = "knut.json";

/*!
 * \qmltype Settings
 * \brief Singleton for accessing and editing persistent settings.
 * \inqmlmodule Knut
 * \ingroup Utilities
 *
 * The settings are stored in a json file, and could be:
 *
 * - bool
 * - int
 * - double
 * - string
 * - array<string>
 */

/*
 * \qmlproperty bool Settings::isTesting
 * Returns true if Knut is currently in a test, and false otherwise
 */

Settings::Settings(Mode mode, QObject *parent)
    : QObject(parent)
    , m_saveTimer(new QTimer(this))
    , m_mode(mode)
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;

    loadKnutSettings();
    if (!isTesting()) // Only load if not testing
        loadUserSettings();

    m_saveTimer->callOnTimeout(this, &Settings::saveSettings);
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
    auto userSettings = loadSettings(userFilePath());
    if (userSettings) {
        m_userSettings = userSettings.value();
        m_settings.merge_patch(m_userSettings);
        emit settingsLoaded();
    }
}

void Settings::loadProjectSettings(const QString &rootDir)
{
    m_projectPath = rootDir;
    auto projectSettings = loadSettings(projectFilePath());
    if (projectSettings) {
        m_projectSettings = projectSettings.value();
        m_settings.merge_patch(m_projectSettings);
        emit settingsLoaded();
    }
}

/*!
 * \qmlmethod bool Settings::hasValue(string path)
 * Returns true if the project settings has a settings `path`.
 */
bool Settings::hasValue(QString path) const
{
    LOG("Settings::hasValue", path);

    if (!path.startsWith('/'))
        path.prepend('/');
    return m_settings.contains(nlohmann::json::json_pointer(path.toStdString()));
}

/*!
 * \qmlmethod variant Settings::value(string path, variant defaultValue = null)
 * Returns the value of the settings `path`, or `defaultValue` if the settings does not exist.
 */
QVariant Settings::value(QString path, const QVariant &defaultValue) const
{
    if (defaultValue.isValid())
        LOG("Settings::value", path, defaultValue);
    else
        LOG("Settings::value", path);
    try {
        if (!path.startsWith('/'))
            path.prepend('/');

        // Special cases
        if (path == RcAssetColors || path == RcAssetFlags || path == RcDialogFlags) {
            return static_cast<int>(value<RcDocument::ConversionFlags>(path.toStdString()));
        }

        auto val = m_settings.at(nlohmann::json::json_pointer(path.toStdString()));
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
            if (!val.empty()) {
                if (val[0].is_string())
                    return val.get<QStringList>();
            } else {
                return QStringList();
            }
        }
        spdlog::error("Settings::value {} - can't convert", path);
    } catch (...) {
        spdlog::info("Settings::value {} - accessing non-existing value", path);
    }
    return defaultValue;
}

/*!
 * \qmlmethod variant Settings::setValue(string path, variant value)
 * Adds a new value `value` to the project settings at the given `path`. Returns `true` if the operation succeeded.
 */
bool Settings::setValue(QString path, const QVariant &value)
{
    LOG("Settings::setValue", path, value);

    if (value.isNull()) {
        spdlog::error("Settings::setValue {} in {} - value is null", value.toString(), path);
        return false;
    }

    if (!path.startsWith('/'))
        path.prepend('/');

    auto jsonPath = nlohmann::json::json_pointer(path.toStdString());
    switch (static_cast<QMetaType::Type>(value.typeId())) {
    case QMetaType::Bool:
        m_settings[jsonPath] = value.toBool();
        m_projectSettings[jsonPath] = value.toBool();
        break;
    case QMetaType::Int:
    case QMetaType::LongLong:
        m_settings[jsonPath] = value.toInt();
        m_projectSettings[jsonPath] = value.toInt();
        break;
    case QMetaType::UInt:
    case QMetaType::ULongLong:
        m_settings[jsonPath] = value.toUInt();
        m_projectSettings[jsonPath] = value.toUInt();
        break;
    case QMetaType::Double:
        m_settings[jsonPath] = value.toDouble();
        m_projectSettings[jsonPath] = value.toDouble();
        break;
    case QMetaType::QString:
        m_settings[jsonPath] = value.toString();
        m_projectSettings[jsonPath] = value.toString();
        break;
    case QMetaType::QStringList:
        m_settings[jsonPath] = value.toStringList();
        m_projectSettings[jsonPath] = value.toStringList();
        break;
    default:
        spdlog::error("Settings::setValue {} in {} - value type not handled", value.toString(), path);
        return false;
    }

    emit settingsChanged(path);
    // Asynchronous save
    m_saveTimer->start();
    return true;
}

QString Settings::userFilePath() const
{
    return QDir::homePath() + '/' + SettingsName;
}

QString Settings::projectFilePath() const
{
    return m_projectPath + '/' + SettingsName;
}

QString Settings::logFilePath() const
{
    // Create QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) directory if it does not exist.
    const QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dir.exists())
        QDir().mkdir(dir.path());

    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/knut.log";
}

bool Settings::isTesting() const
{
    return (m_mode == Mode::Test);
}

bool Settings::hasLsp() const
{
    return m_mode == Mode::Test || (m_mode == Mode::Gui && DEFAULT_VALUE(bool, EnableLSP));
}

void Settings::loadKnutSettings()
{
    QFile file(":/core/settings.json");
    if (file.open(QIODevice::ReadOnly))
        m_settings = nlohmann::json::parse(file.readAll().constData());
}

void Settings::saveSettings()
{
    // Don't save settings if testing
    if (isTesting())
        return;

    const auto &settings = isUser() ? m_userSettings : m_projectSettings;
    const auto &filePath = isUser() ? userFilePath() : projectFilePath();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        spdlog::error("Settings::saveSettings {}", filePath);
        return;
    }

    QTextStream stream(&file);
    stream << QString::fromStdString(settings.dump(4, ' ', false));
    emit settingsSaved();
}

bool Settings::isUser() const
{
    return m_projectPath.isEmpty();
}

void Settings::addScriptPath(const QString &path)
{
    updatePaths(path, ScriptPaths, true);
}

void Settings::removeScriptPath(const QString &path)
{
    updatePaths(path, ScriptPaths, false);
}

std::string Settings::dumpJson() const
{
    return m_settings.dump();
}

void Settings::updatePaths(const QString &path, const std::string &json_path, bool add)
{
    const auto pathPointer = nlohmann::json::json_pointer(json_path);
    auto &settings = isUser() ? m_userSettings : m_projectSettings;

    QStringList paths;
    QStringList globalPaths;
    if (settings.contains(pathPointer))
        paths = settings.at(pathPointer).get<QStringList>();
    if (m_settings.contains(pathPointer))
        globalPaths = m_settings.at(nlohmann::json::json_pointer(json_path)).get<QStringList>();

    if (add) {
        paths.push_back(path);
        globalPaths.push_back(path);
    } else {
        paths.removeAll(path);
        globalPaths.removeAll(path);
    }
    settings[nlohmann::json::json_pointer(json_path)] = paths;
    m_settings[nlohmann::json::json_pointer(json_path)] = globalPaths;
    saveSettings();
}

} // namespace Core
