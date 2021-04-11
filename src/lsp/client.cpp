#include "client.h"

#include "utils/log_utils.h"

#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QProcess>
#include <QSignalSpy>
#include <QString>

#include <doctest/doctest.h>
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

Client::Client(const std::string &language, const QString &program, const QStringList &arguments, QObject *parent)
    : QObject(parent)
    , m_program(program)
    , m_arguments(arguments)
{
    const auto serverLogName = language + "_server";
    m_serverLogger = spdlog::get(serverLogName);
    if (!m_serverLogger) {
        m_serverLogger = spdlog::stdout_color_mt(serverLogName);
        m_serverLogger->set_level(spdlog::level::debug);
        m_serverLogger->set_pattern("%v");
    }

    const auto messageLogName = language + "_messages";
    m_messageLogger = spdlog::get(messageLogName);
    if (!m_messageLogger) {
        m_messageLogger = spdlog::basic_logger_st(messageLogName, messageLogName + ".log", true);
        m_messageLogger->set_level(spdlog::level::info);
        m_messageLogger->set_pattern("[LSP   - %H:%M:%S] %v");
    }

    m_process = new QProcess(this);

    connect(m_process, &QProcess::readyReadStandardError, this, &Client::readError);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Client::readOutput);
    connect(m_process, &QProcess::errorOccurred, this, &Client::handleError);
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &Client::exitServer);
}

void Client::start()
{
    m_serverLogger->trace("==> Starting LSP server {}.", m_program.toLatin1());
    m_process->start(m_program, m_arguments);
    if (m_process->waitForStarted())
        initialize();
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
    emit finished();
}

void Client::handleError(int error)
{
    std::string errorName;
    switch (error) {
    case QProcess::FailedToStart:
        errorName = "FailedToStart";
        break;
    case QProcess::Crashed:
        errorName = "Crashed";
        break;
    case QProcess::Timedout:
        errorName = "Timedout";
        break;
    case QProcess::WriteError:
        errorName = "WriteError";
        break;
    case QProcess::ReadError:
        errorName = "ReadError";
        break;
    case QProcess::UnknownError:
        errorName = "UnknownError";
        break;
    }
    m_serverLogger->error("==> LSP server {} raises an error {}", m_program.toLatin1(), errorName);
    emit errorOccured(error);
}

void Client::sendJsonRequest(nlohmann::json jsonRequest)
{
    logMessage("send-request", jsonRequest);
    const auto message = toMessage(jsonRequest);
    m_process->write(message);
}

void Client::sendJsonNotification(nlohmann::json jsonNotification)
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

///////////////////////////////////////////////////////////////////////////////
// Tests
///////////////////////////////////////////////////////////////////////////////
TEST_SUITE("lsp")
{
    TEST_CASE("start and shutdown server")
    {
        Lsp::Client client("cpp", "clangd", {});
        LogSilencer serverLog("cpp_server");
        LogSilencer messagesLog("cpp_messages");

        QSignalSpy initialized(&client, &Lsp::Client::initialized);
        QSignalSpy errorOccured(&client, &Lsp::Client::errorOccured);
        QSignalSpy finished(&client, &Lsp::Client::finished);

        client.start();
        CHECK(errorOccured.count() == 0);
        REQUIRE(initialized.wait(500));
        client.shutdown();
        REQUIRE(finished.wait(500));
    }

    TEST_CASE("requests")
    {
        Lsp::Client client("cpp", "clangd", {});
        LogSilencer serverLog("cpp_server");
        LogSilencer messagesLog("cpp_messages");
        QSignalSpy initialized(&client, &Lsp::Client::initialized);
        QSignalSpy finished(&client, &Lsp::Client::finished);
        client.start();
        REQUIRE(initialized.wait(500));

        SUBCASE("TODO_REQUEST")
        {
            // TODO one SUBCASE per request
        }

        client.shutdown();
        // Nicely close the client
        CHECK(finished.wait(500));
    }
}
