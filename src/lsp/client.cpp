#include "client.h"

#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QProcess>
#include <QString>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <ctime>

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
    const auto serverLogName = (program + "_server").toStdString();
    m_serverLogger = spdlog::get(serverLogName);
    if (!m_serverLogger) {
        m_serverLogger = spdlog::stdout_color_mt(serverLogName);
        m_serverLogger->set_level(spdlog::level::debug);
        m_serverLogger->set_pattern("%v");
    }

    const auto messageLogName = (program + "_messages").toStdString();
    const auto messageLogFile = program + "_messages.log";
    // Cleanup the file before starting
    if (QFile::exists(messageLogFile))
        QFile::remove(messageLogFile);
    m_messageLogger = spdlog::get(messageLogName);
    if (!m_messageLogger) {
        m_messageLogger = spdlog::basic_logger_st(messageLogName, messageLogFile.toStdString());
        m_messageLogger->set_level(spdlog::level::info);
        m_messageLogger->set_pattern("[LSP   - %H:%M:%S] %v");
    }

    m_process = new QProcess(this);

    connect(m_process, &QProcess::readyReadStandardError, this, &Client::readError);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Client::readOutput);
    connect(m_process, &QProcess::started, this, &Client::initialize);
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &Client::exitServer);
}

void Client::start()
{
    m_serverLogger->trace("==> Starting LSP server {}.", m_program.toLatin1());
    m_process->start(m_program, m_arguments);
}

void Client::shutdown()
{
    ShutdownRequest request;
    sendRequest(request, std::bind(&Client::shutdownCallback, this, std::placeholders::_1));
}

void Client::readError()
{
    m_serverLogger->info(m_process->readAllStandardError());
}

void Client::readOutput()
{
    const auto message = parseMessage(m_process->readAllStandardOutput());

    // Check if there is an error
    if (message.contains("error")) {
        auto errorString = message.at("error").at("message").get<std::string>();
        m_serverLogger->error("==> Error response: {}", errorString);
    }

    if (message.contains("id")) {
        const MessageId id = message.at("id").get<MessageId>();
        auto it = m_callbacks.find(id);
        if (it != m_callbacks.end()) {
            logMessage("receive-response", message);
            it->second(message);
        } else {
            logMessage("receive-request", message);
        }
    } else {
        logMessage("receive-notification", message);
    }
}

void Client::initialize()
{
    m_serverLogger->trace("==> LSP server {} started", m_program.toLatin1());

    InitializeRequest request;
    request.id = 1;
    request.params.processId = QCoreApplication::applicationPid();

    sendRequest(request, std::bind(&Client::initializeCallback, this, std::placeholders::_1));
}

void Client::exitServer()
{
    m_serverLogger->trace("==> LSP server {} exited", m_program.toLatin1());
}

void Client::sendRequest(nlohmann::json jsonRequest)
{
    logMessage("send-request", jsonRequest);
    const auto message = toMessage(jsonRequest);
    m_process->write(message);
}

void Client::sendNotification(nlohmann::json jsonNotification)
{
    logMessage("send-notification", jsonNotification);
    const auto message = toMessage(jsonNotification);
    m_process->write(message);
}

void Client::initializeCallback(InitializeRequest::Response response)
{
    if (response.error) {
        // TODO how do we want to handle errors?
        json j = response.error.value();
        m_serverLogger->error(j.dump());
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
        m_serverLogger->error(j.dump());
    } else {
        sendNotification(ExitNotification());
        emit finished();
    }
}

void Client::logMessage(std::string type, const nlohmann::json &message)
{
    json log = {
        {"type", type},
        {"message", message},
        {"timestamp", std::time(0)},
    };
    m_messageLogger->info(log.dump());
    m_messageLogger->flush();
}
}
