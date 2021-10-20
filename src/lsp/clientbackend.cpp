#include "clientbackend.h"

#include "notificationmessage_json.h"
#include "notifications.h"
#include "requestmessage_json.h"
#include "requests.h"
#include "types_json.h"

#include <QBuffer>
#include <QEventLoop>
#include <QString>

#include <spdlog/sinks/basic_file_sink.h>

#include <ctime>

using json = nlohmann::json;

namespace Lsp {

// Create a new LSP message to send
// Return the message to send, with the header + content
static QByteArray toMessage(json content)
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

ClientBackend::ClientBackend(const std::string &language, QString program, QStringList arguments, QObject *parent)
    : QObject(parent)
    , m_program(std::move(program))
    , m_arguments(std::move(arguments))
    , m_process(new QProcess(this))
{
    const auto serverLogName = language + "_server";
    m_serverLogger = spdlog::get(serverLogName);
    if (!m_serverLogger) {
        m_serverLogger = spdlog::basic_logger_mt(serverLogName, serverLogName + ".log", true);
        m_serverLogger->set_level(spdlog::level::debug);
        m_serverLogger->set_pattern("%v");
    }

    const auto messageLogName = language + "_messages";
    m_messageLogger = spdlog::get(messageLogName);
    if (!m_messageLogger) {
        m_messageLogger = spdlog::basic_logger_mt(messageLogName, messageLogName + ".log", true);
        m_messageLogger->set_level(spdlog::level::info);
        m_messageLogger->set_pattern("[LSP   - %H:%M:%S] %v");
    }

    connect(m_process, &QProcess::readyReadStandardError, this, &ClientBackend::readError);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &ClientBackend::readOutput);
    connect(m_process, &QProcess::errorOccurred, this, &ClientBackend::handleError);
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &ClientBackend::handleFinished);
}

ClientBackend::~ClientBackend()
{
    if (m_process->state() == QProcess::NotRunning)
        return;
    m_process->terminate();
    if (m_process->waitForFinished(300) && m_process->state() == QProcess::Running)
        return;
    m_process->kill();
    m_process->waitForFinished(300);
}

bool ClientBackend::start()
{
    m_serverLogger->trace("==> Starting LSP server {}", m_program.toLatin1());
    m_process->start(m_program, m_arguments);
    if (m_process->waitForStarted() && m_process->state() == QProcess::Running)
        return true;
    return false;
}

void ClientBackend::readError()
{
    m_serverLogger->info(m_process->readAllStandardError());
}

void ClientBackend::readOutput()
{
    m_message.addData(m_process->readAllStandardOutput());

    auto message = m_message.getNextMessage();
    while (!message.is_null()) {
        // Check if there is an error
        if (message.contains("error")) {
            auto errorString = message.at("error").at("message").get<std::string>();
            m_serverLogger->error("<== Error response: {}", errorString);
        }

        if (message.contains("id")) {
            const MessageId id = message.at("id").get<MessageId>();
            auto it = m_callbacks.find(id);
            if (it != m_callbacks.end()) {
                logMessage("receive-response", message);
                it->second(std::move(message));
            } else {
                logMessage("receive-request", message);
            }
        } else {
            logMessage("receive-notification", message);
        }
        message = m_message.getNextMessage();
    }
}

void ClientBackend::handleError()
{
    m_serverLogger->error("==> LSP server {} raises an error {}", m_program.toLatin1(),
                          m_process->errorString().toLatin1());
    emit errorOccured(m_process->errorString());
}

void ClientBackend::handleFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_serverLogger->trace("==> Exiting LSP server {} with exit code {}", m_program.toLatin1(), exitCode);
    if (exitStatus != QProcess::CrashExit)
        emit finished();
}

void ClientBackend::sendAsyncJsonRequest(nlohmann::json jsonRequest)
{
    logMessage("send-request", jsonRequest);
    const auto message = toMessage(std::move(jsonRequest));
    m_process->write(message);
}

nlohmann::json ClientBackend::sendJsonRequest(nlohmann::json jsonRequest)
{
    // Wait for the response to be emitted using the QEventLoop trick
    QEventLoop loop;
    connect(this, &ClientBackend::responseEmitted, &loop, [&loop]() {
        loop.exit();
    });
    sendAsyncJsonRequest(std::move(jsonRequest));
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    return m_response;
}

void ClientBackend::sendJsonNotification(nlohmann::json jsonNotification)
{
    logMessage("send-notification", jsonNotification);
    const auto message = toMessage(std::move(jsonNotification));
    m_process->write(message);
}

void ClientBackend::logMessage(std::string type, const nlohmann::json &message)
{
    json log = {
        {"type", type},
        {"message", message},
        {"timestamp", std::time(0)},
    };
    m_messageLogger->info(log.dump());
    m_messageLogger->flush();
}

void ClientBackend::Message::addData(const QByteArray &data)
{
    m_data += data;
}

nlohmann::json ClientBackend::Message::getNextMessage()
{
    // Not enough data yet to read the message
    if (m_length == 0 && !readHeader())
        return {};

    // There is a message
    if (m_length) {
        QBuffer buf;
        buf.setData(m_data);
        if (buf.open(QIODevice::ReadOnly)) {
            auto message = json::parse(buf.read(m_length).constData());
            m_length = 0;
            m_data = m_data.mid(buf.pos());
            return message;
        }
    }
    return {};
}

bool ClientBackend::Message::readHeader()
{
    QBuffer buf;
    buf.setData(m_data);
    int length = 0;
    if (buf.open(QIODevice::ReadOnly)) {
        while (!buf.atEnd()) {
            const QByteArray &headerLine = buf.readLine();

            // There's always an empty line between header and content
            if (headerLine == "\r\n") {
                m_length = length;
                m_data = m_data.mid(buf.pos());
                return true;
            };

            int assignmentIndex = headerLine.indexOf(": ");
            if (assignmentIndex >= 0) {
                const QByteArray &key = headerLine.mid(0, assignmentIndex).trimmed();
                const QByteArray &value = headerLine.mid(assignmentIndex + 2).trimmed();
                if (key == "Content-Length")
                    length = value.toInt();
            }
        }
    }
    return false;
}
}
