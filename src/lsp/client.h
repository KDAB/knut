#pragma once

#include "initialize_request.h"
#include "shutdown_request.h"

#include <QObject>

#include <spdlog/spdlog.h>

#include <functional>
#include <unordered_map>

class QProcess;

namespace Lsp {

class Client : public QObject
{
    Q_OBJECT

public:
    Client(int languages, const QString &program, const QStringList &arguments, QObject *parent = nullptr);

    void start();
    void shutdown();

signals:
    void initialized();
    void finished();

private:
    void readError();
    void readOutput();
    void initialize();
    void exitServer();

    template <typename Request>
    void sendRequest(Request request, typename Request::ResponseCallback callback)
    {
        m_callbacks[request.id] = [this, callback](nlohmann::json j) {
            if (callback) {
                auto response = j.get<Request::Response>();
                if (!response.isValid())
                    m_serverLogger->error("==> Invalid response from server: {}", j.dump());
                callback(response);
            }
        };
        sendRequest(std::move(request));
    }
    void sendRequest(nlohmann::json jsonRequest);
    void sendNotification(nlohmann::json jsonNotification);

    void initializeCallback(InitializeRequest::Response response);
    void shutdownCallback(ShutdownRequest::Response response);

    void logMessage(std::string type, const nlohmann::json &message);

private:
    std::shared_ptr<spdlog::logger> m_serverLogger;
    std::shared_ptr<spdlog::logger> m_messageLogger;
    int m_languages;
    const QString m_program;
    const QStringList m_arguments;
    QProcess *m_process = nullptr;

    std::unordered_map<MessageId, std::function<void(nlohmann::json)>> m_callbacks;
};

}
