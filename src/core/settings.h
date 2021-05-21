#pragma once

#include <utils/json_utils.h>

#include <QObject>
#include <QStringList>
#include <QVariant>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace Core {

/*!
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
        QString program;
        QStringList arguments;
    };

public:
    ~Settings();

    static Settings *instance();

    void loadProjectSettings(const QString &rootDir);

    template <typename T>
    T value(const std::string &path)
    {
        try {
            return m_settings.at(nlohmann::json::json_pointer(path)).get<T>();
        } catch (...) {
            spdlog::warn("[Settings] Error accessing setting value {}", path);
        }
        return {};
    }

    // TODO : Add API to change the project settings, setValue/removeValue
    Q_INVOKABLE bool hasValue(QString path) const;
    Q_INVOKABLE QVariant value(QString path, const QVariant &defaultValue = {}) const;

protected:
    explicit Settings(bool addUserSettings, QObject *parent = nullptr);

private:
    void loadKnutSettings();
    void loadUserSettings();
    void loadSettings(std::string name, const QString &fileName);

private:
    nlohmann::json m_settings;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings::LspServer, program, arguments);

} // namespace Core
