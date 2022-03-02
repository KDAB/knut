#include "knutcore.h"

#include "project.h"
#include "scriptmanager.h"
#include "settings.h"

#include <QApplication>
#include <QTimer>

#include <spdlog/spdlog.h>

#include <core/textdocument.h>

namespace Core {

KnutCore::KnutCore(QObject *parent)
    : QObject(parent)
{
#ifdef QT_DEBUG
    spdlog::set_level(spdlog::level::trace);
#endif

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
    if (!fileName.isEmpty()) {
        Project::instance()->open(fileName);

        // Moving the cursor position to given Line,Column. It will only work if a file(-i) is passed, will do
        // nothing otherwise.
        const int nLine = parser.value("line").toInt();
        const int nColumn = parser.value("column").toInt();
        if (nLine > 0) {
            Core::Document *currDoc = Project::instance()->currentDocument();
            auto tDoc = dynamic_cast<Core::TextDocument *>(currDoc);
            if (tDoc != nullptr) {
                tDoc->gotoLine(nLine, nColumn);
            }
        }
    }

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

    // Added two more parameters(line,column) to the knut commandline.
    parser.addOptions({
        {{"s", "script"}, "Run given script <file> then exit", "file"},
        {{"r", "root"}, "Root <directory> of the project", "directory"},
        {{"i", "input"}, "Open document <file> on startup", "file"},
        {{"l", "line"}, "Line value to set the current cursor position in the passed file", "line"},
        {{"c", "column"}, "Column value to set the current cursor position in the passed file", "column"},
    });
}

void KnutCore::doParse(const QCommandLineParser &parser) const
{
    Q_UNUSED(parser);
}

} // namespace Core
