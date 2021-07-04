#pragma once

#include "document.h"
#include "json_utils.h"

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QVariant>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <optional>

namespace Core {

/**
 * \brief Singleton storing all settings for Knut
 *
 * There are 3 levels of settings:
 * - Knut default settings, stored in the settings.json file in the knut resources
 * - User settins, stored in homePath/.knut
 * - Project settings, stored in projectRootPath/.knut
 *
 * Settings are read in this order, and new settings are replacing old one if it's the same path.
 *
 * Access to settings is using json pointer: https://tools.ietf.org/html/rfc6901
 */
class Settings : public QObject
{
    Q_OBJECT

public:
    //! Store settings relative to a LSP server
    struct LspServer
    {
        Document::Type type;
        QString program;
        QStringList arguments;
    };

    static inline const char RcDialogFlags[] = "/rc/dialog_flags";
    static inline const char RcDialogScaleX[] = "/rc/dialog_scalex";
    static inline const char RcDialogScaleY[] = "/rc/dialog_scaley";
    static inline const char RcAssetFlags[] = "/rc/asset_flags";
    static inline const char RcAssetColors[] = "/rc/asset_transparent_colors";
    static inline const char ScriptPaths[] = "/script_paths";

public:
    ~Settings();

    static Settings *instance();

    void loadUserSettings();
    void loadProjectSettings(const QString &rootDir);

    void addScriptPath(const QString &path);
    void removeScriptPath(const QString &path);

    template <typename T>
    T value(std::string path)
    {
        if (!path.starts_with('/'))
            path = '/' + path;
        try {
            return m_settings.at(nlohmann::json::json_pointer(path)).get<T>();
        } catch (...) {
            spdlog::error("Error accessing setting value {}", path);
        }
        return {};
    }

    template <typename T>
    bool setValue(std::string path, const T &value)
    {
        if (!path.starts_with('/'))
            path = '/' + path;
        try {
            m_settings[nlohmann::json::json_pointer(path)] = value;
            m_projectSettings[nlohmann::json::json_pointer(path)] = value;
        } catch (...) {
            spdlog::error("Error saving setting value {}", path);
            return false;
        }
        m_saveTimer->start();
        return true;
    }

    Q_INVOKABLE bool hasValue(QString path) const;
    Q_INVOKABLE QVariant value(QString path, const QVariant &defaultValue = {}) const;

    Q_INVOKABLE bool setValue(QString path, const QVariant &value);

    QString userPath() const;
    QString projectPath() const;

signals:
    void projectSettingsSaved();

protected:
    Settings(QObject *parent = nullptr);

private:
    friend class KnutCore;

    void loadKnutSettings();
    std::optional<nlohmann::json> loadSettings(const QString &name, bool log = true);
    void saveSettings(const QString &name, const nlohmann::json &settings);
    void addScriptPaths(const nlohmann::json &settings);
    void updateScriptPaths(const QString &path, bool add);

private:
    inline static Settings *m_instance = nullptr;

    nlohmann::json m_settings;
    nlohmann::json m_projectSettings;
    QString m_userPath;
    QString m_projectPath;
    QTimer *m_saveTimer = nullptr;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings::LspServer, type, program, arguments);

} // namespace Core

#define DEFAULT_VALUE(Type, PATH) Core::Settings::instance()->value<Type>(Core::Settings::PATH)
#define SET_DEFAULT_VALUE(PATH, Value) Core::Settings::instance()->setValue(Core::Settings::PATH, Value)
