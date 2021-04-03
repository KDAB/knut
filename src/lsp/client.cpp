#include "client.h"

#include <QApplication>
#include <QBuffer>
#include <QProcess>
#include <QString>

#include <spdlog/sinks/stdout_color_sinks.h>

using json = nlohmann::json;

namespace Lsp {

// Create a new LSP message to send
// Return the message to send, with the header + content
static QByteArray toMessage(const json &content)
{
    QByteArray data = QByteArray::fromStdString(content.dump());
    const int length = data.size();

    // https://microsoft.github.io/language-server-protocol/specifications/specification-current/#headerPart
    // The content-type is optional, and only UTF-8 is accepted for the charset
    // Content-Length: ...\r\n
    // Content-Type: application/vscode-jsonrpc; charset=utf-8\r\n
    // \r\n
    // {
    //     ~~~
    // }
    const QString header = QString("Content-Length: %1\r\n\r\n").arg(length);

    return header.toLatin1() + data;
}

// Parse the given LSP message
// Return the content of the message as a json object or empty if it's invalid
static json parseMessage(const QByteArray &message)
{
    QBuffer buf;
    buf.setData(message);
    int length = 0;
    if (buf.open(QIODevice::ReadOnly)) {
        while (!buf.atEnd()) {
            const QByteArray &headerLine = buf.readLine();

            // There's always an empty line between header and content
            if (headerLine == "\r\n")
                break;

            int assignmentIndex = headerLine.indexOf(": ");
            if (assignmentIndex >= 0) {
                const QByteArray &key = headerLine.mid(0, assignmentIndex).trimmed();
                const QByteArray &value = headerLine.mid(assignmentIndex + 2).trimmed();
                if (key == "Content-Length")
                    length = value.toInt();
            }
        }
    }

    if (length)
        return json::parse(buf.read(length).constData());
    return {};
}

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
    connect(m_process, &QProcess::started, this, &Client::initialize);
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &Client::exitServer);
}

void Client::start()
{
    m_logger->info("Starting LSP server {}.", m_program.toLatin1());
    m_process->start(m_program, m_arguments);
}

void Client::shutdown()
{
    ShutdownRequest request;
    sendRequest(request, std::bind(&Client::shutdownCallback, this, std::placeholders::_1));
}

void Client::readError()
{
    m_logger->info(m_process->readAllStandardError());
}

void Client::readOutput()
{
    const auto message = parseMessage(m_process->readAllStandardOutput());
    m_logger->trace("<== {}", message.dump());
    // Check if there is an error
    if (message.contains("error")) {
        auto errorString = message.at("error").at("message").get<std::string>();
        m_logger->error(errorString);
    }

    if (message.contains("id")) {
        const MessageId id = message.at("id").get<MessageId>();
        auto it = m_callbacks.find(id);
        if (it != m_callbacks.end()) {
            it->second(message);
        }
    }
}

void Client::initialize()
{
    m_logger->info("LSP server {} started", m_program.toLatin1());

    InitializeRequest request;
    request.id = 1;
    request.params.processId = QCoreApplication::applicationPid();

    sendRequest(request, std::bind(&Client::initializeCallback, this, std::placeholders::_1));
}

void Client::exitServer()
{
    m_logger->info("LSP server {} exited", m_program.toLatin1());
}

void Client::sendRequest(nlohmann::json jsonRequest)
{
    m_logger->trace("==> {}", jsonRequest.dump());

    const auto message = toMessage(jsonRequest);
    m_process->write(message);
}

void Client::sendNotification(nlohmann::json jsonRequest)
{
    m_logger->trace("==> {}", jsonRequest.dump());

    const auto message = toMessage(jsonRequest);
    m_process->write(message);
}

void Client::initializeCallback(InitializeRequest::Response response)
{
    if (response.error) {
        // TODO how do we want to handle errors?
        json j = response.error.value();
        m_logger->error(j.dump());
    } else if (response.result) {
        // TODO initialize some client internal flags
        sendNotification(InitializedNotification());
        emit initialized();
    }
}

void Client::shutdownCallback(ShutdownRequest::Response response)
{
    if (response.error) {
        // TODO how do we want to handle errors?
        json j = response.error.value();
        m_logger->error(j.dump());
    } else {
        sendNotification(ExitNotification());
        emit finished();
    }
}
}
