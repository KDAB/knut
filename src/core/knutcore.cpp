/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "knutcore.h"
#include "project.h"
#include "scriptmanager.h"
#include "settings.h"
#include "textdocument.h"
#include "utils/log.h"

#include <QApplication>
#include <QDir>
#include <QTimer>
#include <spdlog/cfg/env.h>

namespace Core {

KnutCore::KnutCore(QObject *parent)
    : KnutCore({}, parent)
{
    // If KnutCore is created directly, it means that we are in a test
    // Just initialize all singletons here
    initialize(true);
}

KnutCore::KnutCore(InternalTag, QObject *parent)
    : QObject(parent)
{
#ifdef QT_DEBUG
    spdlog::set_level(spdlog::level::trace);
#endif

    spdlog::cfg::load_env_levels();
}

void KnutCore::process(const QStringList &arguments)
{
    // Parse command line options
    QCommandLineParser parser;
    initParser(parser);
    parser.process(arguments);

    const bool isTesting = parser.isSet("test");
    initialize(isTesting);

    const QStringList positionalArguments = parser.positionalArguments();
    // Set the root directory
    if (!positionalArguments.isEmpty()) {
        const QString rootDir = positionalArguments.at(0);
        const QDir pathDir(rootDir);
        if (pathDir.exists()) {
            Project::instance()->setRoot(rootDir);
        } else {
            spdlog::error("KnutCore::process - Root directory: {}, does not exist. Cannot open a new project!",
                          pathDir.absolutePath());
        }
    }

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
            auto tDoc = qobject_cast<Core::TextDocument *>(currDoc);
            if (tDoc != nullptr) {
                tDoc->gotoLine(nLine, nColumn);
            }
        }
    }

    // Run the script passed in parameter, if any
    // Exit Knut if there are no windows opened
    auto scriptName = parser.value("run");
    if (scriptName.isEmpty()) {
        scriptName = parser.value("test");
    }

    if (!scriptName.isEmpty()) {
        QTimer::singleShot(0, this, [scriptName]() {
            ScriptManager::instance()->runScript(scriptName);
        });
        connect(
            ScriptManager::instance(), &ScriptManager::scriptFinished, qApp,
            [](const QVariant &value) {
                qApp->exit(value.toInt());
            },
            Qt::QueuedConnection);
        return;
    }

    doParse(parser);
}

void KnutCore::initParser(QCommandLineParser &parser) const
{
    parser.setApplicationDescription("Automation tool for code transformation using scripts");
    parser.addPositionalArgument(QStringLiteral("project"), QStringLiteral("The project directory."));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({{{"r", "run"}, "Runs given script <file> then exit.", "file"},
                       {{"t", "test"}, "Tests given script <file> then exit.", "file"},
                       {{"i", "input"}, "Opens document <file> on startup.", "file"},
                       {{"l", "line"}, "Line in the current file, if any.", "line"},
                       {{"c", "column"}, "Column in the current file, if any.", "column"}});
}

void KnutCore::doParse(const QCommandLineParser &parser) const
{
    Q_UNUSED(parser)
}

void KnutCore::initialize(bool isTesting)
{
    // Make sure we initialize only once, double initialization could happen in tests
    // If creating a KnutCore and then processing command line arguments
    if (m_initialized)
        return;
    new Settings(isTesting, this);
    new Project(this);
    new ScriptManager(this);
    m_initialized = true;
}

} // namespace Core
