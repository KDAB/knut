#include "knutcore.h"

#include "project.h"
#include "scriptmanager.h"
#include "settings.h"

#include <QCoreApplication>

namespace Core {

KnutCore::KnutCore(QObject *parent)
    : QObject(parent)
{
    // Initialize some singletons
    new Project(this);
    new Settings(this);
    new ScriptManager(this);
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

    // Run the script passed in parameter, if any
    const QString scriptName = parser.value("script");
    if (!scriptName.isEmpty())
        ScriptManager::instance()->runScript(scriptName);

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
    });
}

void KnutCore::doParse(const QCommandLineParser &parser) const
{
    Q_UNUSED(parser);
}

} // namespace Core
