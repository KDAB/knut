#include "client.h"

#include <QProcess>

#include <spdlog/sinks/stdout_color_sinks.h>

using namespace Lsp;

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
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &Client::finishProcess);
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
    m_logger->trace(m_process->readAllStandardError());
}

void Client::finishProcess()
{
    m_logger->info("Exiting LSP server {}.", m_program.toLatin1());
}
