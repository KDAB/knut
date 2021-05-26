#include "knutmain.h"

#include "core/scriptmanager.h"
#include "core/settings.h"

#include <QCommandLineParser>
#include <QCoreApplication>

namespace Ui {

KnutMain::KnutMain(QObject *parent)
    : QObject(parent)
{
    // Initialize some singletons
    new Core::Settings(this);
    new Core::ScriptManager(this);
}

void KnutMain::process(const QCoreApplication &app)
{
    process(app.arguments());
}

void KnutMain::process(const QStringList &arguments)
{
    // Parse command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("Automation tool for source code editing");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
        {{"s", "script"}, "Run given script then exit", "file"},
        {{"r", "root"}, "Root directory of the project", "directory"},
    });

    parser.process(arguments);

    // Set the root directory
    const QString rootDir = parser.value("root");
    if (!rootDir.isEmpty())
        Core::Settings::instance()->loadProjectSettings(rootDir);

    // Run the script passed in parameter, if any
    const QString scriptName = parser.value("script");
    if (!scriptName.isEmpty())
        Core::ScriptManager::instance()->runScript(scriptName);
}

}
