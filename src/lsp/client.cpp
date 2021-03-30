#include "client.h"

#include "message.h"

#include <QProcess>

#include <nlohmann/json.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
using json = nlohmann::json;

namespace Lsp {

Client::Client(const QString &program, const QStringList &arguments, QObject *parent)
    : QObject(parent)
    , m_program(program)
    , m_arguments(arguments)
{
    m_logger = spdlog::get("LSP_Client");
    if (!m_logger)
        m_logger = spdlog::stdout_color_mt("LSP_Client");
    m_logger->set_level(spdlog::level::trace);

    m_process = new QProcess(this);

    connect(m_process, &QProcess::readyReadStandardError, this, &Client::readError);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Client::readOutput);
    connect(m_process, &QProcess::started, this, &Client::initializeServer);
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &Client::exitServer);
}

void Client::start()
{
    m_logger->info("Starting LSP server {}.", m_program.toLatin1());
    m_process->start(m_program, m_arguments);
}

void Client::readError()
{
    m_logger->info(m_process->readAllStandardError());
}

void Client::readOutput()
{
    const auto message = Lsp::parseMessage(m_process->readAllStandardOutput());
    m_logger->trace("<== {}", message);
}

void Client::initializeServer()
{
    m_logger->info("LSP server {} started", m_program.toLatin1());

    const auto initializeJson = R"(
           {
               "jsonrpc": "2.0",
               "id": 1,
               "method": "initialize",
               "params": {}
           }
       )"_json.dump();
    m_logger->trace("==> {}", initializeJson);

    const auto message = Lsp::toMessage(QByteArray::fromStdString(initializeJson));
    m_process->write(message);
}

void Client::exitServer()
{
    m_logger->info("LSP server {} exited", m_program.toLatin1());
}

}
