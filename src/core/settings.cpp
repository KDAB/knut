#include "settings.h"

#include <QDir>
#include <QFile>
#include <QMessageBox>

using json = nlohmann::json;

namespace Core {

static const char settingsName[] = ".knut";

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
        } catch (...) {
            logWarning(QString("Error loading the project settings, in file:\n%1").arg(fileName));
        }
    }
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
        } catch (...) {
            logWarning(QString("Error loading the user settings, in file:\n%1").arg(fileName));
        }
    }
}

void Settings::logWarning(const QString &text) const
{
    QMessageBox::warning(nullptr, "Knut", text);
}

} // namespace Core
