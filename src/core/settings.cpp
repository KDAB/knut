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

namespace Core {

static constexpr char SettingsName[] = "knut.json";

/*!
 * \qmltype Settings
 * \brief Singleton for accessing and editing persistent settings.
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
    saveOnExit();
    m_instance = nullptr;
}

Settings *Settings::instance()
{
    Q_ASSERT(m_instance);
    return m_instance;
}

void Settings::loadUserSettings()
{
    const QString fileName = userFilePath();

    auto loadJsonDataStatus = Utils::loadJsonData(fileName);
    triggerLog(loadJsonDataStatus.loadJsonStatus, fileName, "Settings::loadUserSettings");

    if (loadJsonDataStatus.jsonData) {
        m_userSettings = loadJsonDataStatus.jsonData.value();
        m_settings.merge_patch(m_userSettings);
        emit settingsLoaded();
    }
}

void Settings::loadProjectSettings(const QString &rootDir)
{
    m_projectPath = rootDir;

    const QString fileName = projectFilePath();

    auto loadJsonDataStatus = Utils::loadJsonData(fileName);
    triggerLog(loadJsonDataStatus.loadJsonStatus, fileName, "Settings::loadProjectSettings");

    if (loadJsonDataStatus.jsonData) {
        m_projectSettings = loadJsonDataStatus.jsonData.value();
        m_settings.merge_patch(m_projectSettings);
        emit settingsLoaded();
    }
}

void Settings::triggerLog(const Utils::LoadJsonStatus &loadJsonStatus, const QString &fileName, const QString &caller)
{
    switch (loadJsonStatus) {
    case Utils::LoadJsonStatus::Success:
        break;
    case Utils::LoadJsonStatus::InvalidJson:
        spdlog::error("{} {} - Invalid Json", caller, fileName);
        break;
    case Utils::LoadJsonStatus::UnreadableFile:
        spdlog::error("{} {} - File is not readable", caller, fileName);
        break;
    case Utils::LoadJsonStatus::UnknownError:
        spdlog::error("{} {} - Unknown error", caller, fileName);
        break;
    }
}

/*!
 * \qmlmethod bool Settings::hasValue(string path)
 * Returns true if the project settings has a settings `path`.
 */
bool Settings::hasValue(const QString &path) const
{
    LOG("Settings::hasValue", path);
    return Utils::hasJsonValue(m_settings, path);
}

/*!
 * \qmlmethod variant Settings::value(string path, variant defaultValue = null)
 * Returns the value of the settings `path`, or `defaultValue` if the settings does not exist.
 */
QVariant Settings::value(const QString &path, const QVariant &defaultValue) const
{
    if (defaultValue.isValid())
        LOG("Settings::value", path, defaultValue);
    else
        LOG("Settings::value", path);

    // Special cases
    if (path == Settings::RcAssetColors || path == Settings::RcAssetFlags || path == Settings::RcDialogFlags) {
        return static_cast<int>(value<RcDocument::ConversionFlags>(path.toStdString()));
    }

    const Utils::GetJsonValueStatus valueStatus = Utils::jsonValue(m_settings, path, defaultValue);

    switch (valueStatus.jsonValueStatus) {
    case Utils::JsonValueStatus::Success:
        return valueStatus.value;
    case Utils::JsonValueStatus::ValueUnknown:
        spdlog::info("Settings::value {} - accessing non-existing value", path);
        break;
    case Utils::JsonValueStatus::ConversionUnknown:
        spdlog::error("Settings::value {} - cannot convert", path);
        break;
    }
    return defaultValue;
}

/*!
 * \qmlmethod variant Settings::setValue(string path, var value)
 * Adds a new value `value` to the project settings at the given `path`. Returns `true` if the operation succeeded.
 */
bool Settings::setValue(const QString &path, const QJSValue &value)
{
    const auto var = value.toVariant();

    LOG("Settings::setValue", path, var);

    // Handle both settings and project settings
    Utils::SetJsonValueStatus status = Utils::setJsonValue(m_settings, path, var);
    Utils::SetJsonValueStatus projectSettingsStatus = Utils::setJsonValue(m_projectSettings, path, var);
    // Make sure their statuses are identicals
    Q_ASSERT(status == projectSettingsStatus);

    switch (status) {
    case Utils::SetJsonValueStatus::Success: {
        emit settingsChanged(path);
        // Asynchronous save
        m_saveTimer->start();
        return true;
    }
    case Utils::SetJsonValueStatus::NullValue:
        spdlog::error("Settings::setValue {} in {} - value is null", value.toString(), path);
        break;
    case Utils::SetJsonValueStatus::ValueTypeNotHandled:
        spdlog::error("Settings::setValue {} in {} - value type not handled", value.toString(), path);
        break;
    }
    return false;
}

QString Settings::userFilePath() const
{
    if (isTesting()) {
        return QDir::tempPath() + '/' + SettingsName;
    }
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

void Settings::saveOnExit()
{
    if (m_saveTimer->isActive()) {
        saveSettings();
    }
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
