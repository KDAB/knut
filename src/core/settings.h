/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "document.h"
#include "utils/json.h"
#include "utils/log.h"

#include <QJSValue>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QVariant>

namespace Core {

/**
 * \brief Singleton storing all settings for Knut
 *
 * There are 3 levels of settings:
 * - Knut default settings, stored in the settings.json file in the knut resources
 * - User settings, stored in homePath/.knut
 * - Project settings, stored in projectRootPath/.knut
 *
 * Settings are read in this order, and new settings are replacing old one if it's the same path.
 *
 * Access to settings is using json pointer: https://tools.ietf.org/html/rfc6901
 */
class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isTesting READ isTesting CONSTANT)

public:
    static inline constexpr char EnableLSP[] = "/lsp/enabled";
    static inline constexpr char MimeTypes[] = "/mime_types";
    static inline constexpr char LspServers[] = "/lsp/servers";
    static inline constexpr char RcDialogFlags[] = "/rc/dialog_flags";
    static inline constexpr char RcDialogScaleX[] = "/rc/dialog_scalex";
    static inline constexpr char RcDialogScaleY[] = "/rc/dialog_scaley";
    static inline constexpr char RcAssetFlags[] = "/rc/asset_flags";
    static inline constexpr char RcAssetColors[] = "/rc/asset_transparent_colors";
    static inline constexpr char RcLanguageMap[] = "/rc/language_map";
    static inline constexpr char SaveLogsToFile[] = "/logs/saveToFile";
    static inline constexpr char ScriptPaths[] = "/script_paths";
    static inline constexpr char Tab[] = "/text_editor/tab";
    static inline constexpr char ToggleSection[] = "/toggle_section";

public:
    ~Settings() override;

    static Settings *instance();

    void loadProjectSettings(const QString &rootDir);

    void addScriptPath(const QString &path);
    void removeScriptPath(const QString &path);

    [[nodiscard]] std::string dumpJson() const;

    template <typename T>
    T value(std::string path) const
    {
        if (!path.starts_with('/'))
            path = '/' + path;
        try {
            return m_settings.at(nlohmann::json::json_pointer(path)).get<T>();
        } catch (...) {
            spdlog::error("Settings::value {} - error reading", path);
        }
        return {};
    }

    template <typename T>
    bool setValue(std::string path, const T &value)
    {
        if (!path.starts_with('/'))
            path = '/' + path;
        try {
            const auto pointer = nlohmann::json::json_pointer(path);
            m_settings[pointer] = value;
            if (isUser())
                m_userSettings[pointer] = value;
            else
                m_projectSettings[pointer] = value;
            emit settingsChanged(QString::fromStdString(path));
        } catch (...) {
            spdlog::error("Settings::setValue {} - error saving", path);
            return false;
        }
        m_saveTimer->start();
        return true;
    }

    Q_INVOKABLE bool hasValue(QString path) const;
    Q_INVOKABLE QVariant value(QString path, const QVariant &defaultValue = {}) const;

    QString userFilePath() const;
    QString projectFilePath() const;
    QString logFilePath() const;

    bool isTesting() const;
    bool hasLsp() const;

public slots:
    bool setValue(QString path, const QJSValue &value);

signals:
    void settingsLoaded();
    void settingsChanged(const QString &path);
    void settingsSaved();

protected:
    enum class Mode {
        Test,
        Cli,
        Gui,
    };

    Settings(Mode mode, QObject *parent = nullptr);

private:
    friend class KnutCore;

    void loadKnutSettings();
    void loadUserSettings();
    void updatePaths(const QString &path, const std::string &json_path, bool add);
    void saveSettings();
    bool isUser() const;

    inline static Settings *m_instance = nullptr;

    nlohmann::json m_settings;
    nlohmann::json m_userSettings;
    nlohmann::json m_projectSettings;
    QString m_projectPath;
    QTimer *m_saveTimer = nullptr;
    Mode m_mode = Mode::Test;
};

} // namespace Core

#define DEFAULT_VALUE(Type, PATH) Core::Settings::instance()->value<Type>(Core::Settings::PATH)
#define SET_DEFAULT_VALUE(PATH, Value) Core::Settings::instance()->setValue(Core::Settings::PATH, Value)
