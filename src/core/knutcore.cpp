#include "knutcore.h"

#include "project.h"
#include "scriptmanager.h"
#include "settings.h"

#include <QApplication>
#include <QTimer>

namespace Core {

KnutCore::KnutCore(QObject *parent)
    : QObject(parent)
{
    // Initialize some singletons
    new Project(this);
    auto settings = new Settings(this);
    new ScriptManager(this);

    // Need to be done once ScriptManager exists
    settings->loadUserSettings();
}

void KnutCore::process(const QCoreApplication &app)
{
    process(app.arguments());
}

void KnutCore::process(const QStringList &arguments)
{
    // Parse command line options
    QCommandLineParser parser;
    initParser(parser);
    parser.process(arguments);

    // Set the root directory
    const QString rootDir = parser.value("root");
    if (!rootDir.isEmpty())
        Project::instance()->setRoot(rootDir);

    // Open document on startup
    const QString fileName = parser.value("input");
    if (!fileName.isEmpty())
        Project::instance()->open(fileName);

    // Run the script passed in parameter, if any
    // Exit Knut if there are no windows opened
    const QString scriptName = parser.value("script");
    if (!scriptName.isEmpty()) {
        QTimer::singleShot(0, this, [scriptName]() {
            ScriptManager::instance()->runScript(scriptName);
        });
    }

    doParse(parser);
}

void KnutCore::initParser(QCommandLineParser &parser) const
{
    parser.setApplicationDescription("Automation tool for source code editing");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
        {{"s", "script"}, "Run given script <file> then exit", "file"},
        {{"r", "root"}, "Root <directory> of the project", "directory"},
        {{"i", "input"}, "Open document <file> on startup", "file"},
    });
}

void KnutCore::doParse(const QCommandLineParser &parser) const
{
    Q_UNUSED(parser);
}

} // namespace Core
