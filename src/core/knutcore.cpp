/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "knutcore.h"
#include "project.h"
#include "scriptmanager.h"
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
    initialize(Settings::Mode::Test);
}

KnutCore::KnutCore(InternalTag, QObject *parent)
    : QObject(parent)
{
#ifdef QT_DEBUG
    spdlog::set_level(spdlog::level::trace);
#endif

    spdlog::cfg::load_env_levels();
}

bool KnutCore::process(const QStringList &arguments)
{
    // Parse command line options
    QCommandLineParser parser;
    initParser(parser);
    parser.process(arguments);

    const bool jsonList = parser.isSet("json-list");
    if (jsonList) {
        initialize(Settings::Mode::Cli);
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

    const bool jsonSettings = parser.isSet("json-settings");
    if (jsonSettings) {
        initialize(Settings::Mode::Cli);
        std::cout << Core::Settings::instance()->dumpJson() << "\n";
        exit(0);
    }

    Settings::Mode mode;
    if (parser.isSet("test"))
        mode = Settings::Mode::Test;
    else if (parser.isSet("run"))
        mode = Settings::Mode::Cli;
    else
        mode = Settings::Mode::Gui;

    // Finish scripts using scriptFinished(), not just when the last window is closed
    if (mode == Settings::Mode::Test)
        QApplication::setQuitOnLastWindowClosed(false);

    initialize(mode);

    const QStringList positionalArguments = parser.positionalArguments();
    // Set the root directory
    if (!positionalArguments.isEmpty()) {
        const QString &rootDir = positionalArguments.at(0);
        const QDir pathDir(rootDir);
        if (pathDir.exists()) {
            Project::instance()->setRoot(rootDir);
        } else {
            spdlog::error("{} - Root directory: {}, does not exist. Cannot open a new project!", FUNCTION_NAME,
                          pathDir.absolutePath());
            return false;
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

    // Get json data if provided
    const QString jsonDataStr = parser.value("data");
    json jsonData;
    if (!jsonDataStr.isEmpty()) {
        try {
            jsonData = json::parse(jsonDataStr.toStdString());
        } catch (const json::parse_error &ex) {
            spdlog::error("{}: JSON parsing error at byte {}: {}", FUNCTION_NAME, ex.byte, ex.what());
            return false;
        }
    }

    // Run the script passed in parameter, if any
    // Exit Knut if there are no windows opened
    auto scriptName = parser.value("run");
    if (scriptName.isEmpty()) {
        scriptName = parser.value("test");
    }

    if (!scriptName.isEmpty()) {
        QTimer::singleShot(0, this, [scriptName, jsonData = std::move(jsonData)]() mutable {
            ScriptManager::instance()->runScript(scriptName, std::move(jsonData));
        });
        connect(
            ScriptManager::instance(), &ScriptManager::scriptFinished, qApp,
            [](const QVariant &value) {
                qApp->exit(value.toInt());
            },
            Qt::QueuedConnection);
        return true;
    }

    doParse(parser);
    return true;
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
                       {{"d", "data"}, "JSON data string for initializing the dialog.", "data"},
                       {"json-list", "Returns the list of all available scripts as a JSON file"},
                       {"json-settings", "Returns the settings as a JSON file"}});
}

void KnutCore::doParse(const QCommandLineParser &parser) const
{
    Q_UNUSED(parser)
}

void KnutCore::initialize(Settings::Mode mode)
{
    // Make sure we initialize only once, double initialization could happen in tests
    // If creating a KnutCore and then processing command line arguments
    if (m_initialized)
        return;
    new Settings(mode, this);
    new Project(this);
    new ScriptManager(this);
    if (Core::Settings::instance()->value<bool>(Core::Settings::SaveLogsToFile))
        initializeMultiSinkLogger();
    // auto flush when "info" or higher message is logged.
    spdlog::flush_on(spdlog::level::info);
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
}

} // namespace Core
