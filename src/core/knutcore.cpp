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

#include <QAbstractItemModel>
#include <QApplication>
#include <QDir>
#include <QTimer>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/rotating_file_sink.h>

using json = nlohmann::json;

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

    const bool list = parser.isSet("json-list");
    if (list) {
        initialize(false);
        auto model = Core::ScriptManager::model();
        if (model->rowCount() == 0) {
            std::cout << "[]\n";
        } else {
            json outputJson;
            for (int i = 0; i < model->rowCount(); ++i) {
                const auto name = model->data(model->index(i, 0)).toString().toStdString();
                const auto description = model->data(model->index(i, 1)).toString().toStdString();
                const auto path = model->data(model->index(i, 0), Qt::UserRole).toString().toStdString(); // PathRole
                outputJson.push_back({{"name", name}, {"description", description}, {"path", path}});
            }
            std::cout << outputJson.dump() << "\n";
        }

        exit(0);
    }

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
                       {{"c", "column"}, "Column in the current file, if any.", "column"},
                       {"json-list", "Lists all available scripts"}});
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
    if (Core::Settings::instance()->value<bool>(Core::Settings::SaveLogsToFile))
        initializeMultiSinkLogger();
    m_initialized = true;
}

void KnutCore::initializeMultiSinkLogger()
{
    // Define fileLogger arguments (make it clear)
    constexpr int max_files = 5;
    constexpr bool rotate_on_open = true;

    // Create a default Knut logger that save the logs to file.
    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        Core::Settings::instance()->logFilePath().toStdString(), SIZE_MAX, max_files, rotate_on_open);
    auto logger = spdlog::default_logger();
    logger->sinks().push_back(fileSink);
    // auto flush when "info" or higher message is logged.
    spdlog::flush_on(spdlog::level::info);
}

} // namespace Core
