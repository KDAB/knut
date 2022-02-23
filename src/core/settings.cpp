#include "settings.h"

#include "project.h"
#include "rcdocument.h"
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
        saveSettings(m_projectPath, m_projectSettings);
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
    m_userPath = QDir::homePath() + '/' + SettingsName;
    auto userSettings = loadSettings(m_userPath);
    if (userSettings) {
        m_settings.merge_patch(userSettings.value());
        addScriptPaths(userSettings.value());
    }
}

void Settings::loadProjectSettings(const QString &rootDir)
{
    m_projectPath = rootDir + '/' + SettingsName;
    auto userSettings = loadSettings(m_projectPath);
    if (userSettings) {
        m_projectSettings = userSettings.value();
        m_settings.merge_patch(m_projectSettings);
        addScriptPaths(m_projectSettings);
    }
}

/*!
 * \qmlmethod bool ProjectSettings::hasValue( string path)
 * Returns true if the project settings has a settings `path`.
 */
bool Settings::hasValue(QString path) const
{
    spdlog::trace("Settings::hasValue {}", path.toStdString());

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
        spdlog::trace("Settings::value {}", path.toStdString());

        if (!path.startsWith('/'))
            path.prepend('/');

        // Special cases
        if (path == RcAssetColors || path == RcAssetFlags || path == RcDialogFlags) {
            return static_cast<int>(value<RcDocument::ConversionFlags>(path.toStdString()));
        }

        auto val = m_settings.at(json::json_pointer(path.toStdString()));
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
        spdlog::error("Settings::value {} - can't convert", path.toStdString());
    } catch (...) {
        spdlog::info("Settings::value {} - accessing non-existing value", path.toStdString());
    }
    return defaultValue;
}

/*!
 * \qmlmethod variant Settings::setValue( string path, variant value)
 * Adds a new value `value` to the project settings at the given `path`. Returns `true` if the operation succeeded.
 */
bool Settings::setValue(QString path, const QVariant &value)
{
    spdlog::trace("Settings::setValue {} in {}", value.toString().toStdString(), path.toStdString());

    if (value.isNull()) {
        spdlog::error("Settings::setValue {} in {} - value is null", value.toString().toStdString(),
                      path.toStdString());
        return false;
    }

    if (!path.startsWith('/'))
        path.prepend('/');

    auto jsonPath = json::json_pointer(path.toStdString());
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    switch (static_cast<QMetaType::Type>(value.type())) {
#else
    switch (static_cast<QMetaType::Type>(value.typeId())) {
#endif
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
        spdlog::error("Settings::setValue {} in {} - value type not handled", value.toString().toStdString(),
                      path.toStdString());
        return false;
    }
    // Asynchronous save
    m_saveTimer->start();
    return true;
}

QString Settings::userPath() const
{
    return m_userPath;
}

QString Settings::projectPath() const
{
    return m_projectPath;
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
                spdlog::trace("Settings::loadSettings {}", name.toStdString());
            return settings;
        } catch (...) {
            if (log)
                spdlog::error("Settings::loadSettings {}", name.toStdString());
        }
    } else {
        if (log)
            spdlog::debug("Settings::loadSettings {} - file can't be read", name.toStdString());
        return "{}"_json;
    }
    return {};
}

void Settings::saveSettings(const QString &name, const nlohmann::json &settings)
{
    if (name.isEmpty()) {
        spdlog::error("Settings::saveSettings - no project loaded");
        return;
    }

    QFile file(name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        spdlog::error("Settings::saveSettings {}", name.toStdString());
        return;
    }

    QTextStream stream(&file);
    stream << QString::fromStdString(settings.dump(4, ' ', false));
    if (name == m_projectPath)
        emit projectSettingsSaved();
}

void Settings::addScriptPaths(const nlohmann::json &settings)
{
    // Load script paths if any
    try {
        auto scriptPaths = settings.at(nlohmann::json::json_pointer(ScriptPaths)).get<QStringList>();
        for (const auto &path : scriptPaths)
            ScriptManager::instance()->addDirectory(path);
    } catch (...) {
    }
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

    const QString name = isUSer ? m_userPath : m_projectPath;
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
