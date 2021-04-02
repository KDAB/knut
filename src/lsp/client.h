#pragma once

#include "initialize_message.h"

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
    Client(const QString &program, const QStringList &arguments, QObject *parent = nullptr);

    void start();

    template <typename Request>
    void sendRequest(Request request, typename Request::ResponseCallback callback)
    {
        m_callbacks[request.id] = [this, callback](nlohmann::json j) {
            if (callback) {
                auto response = j.get<Request::Response>();
                if (!response.isValid())
                    m_logger->error("Invalid response from server: {}", j.dump());
                callback(response);
            }
        };
        sendRequest(std::move(request));
    }

private:
    // TODO extract the QProcess specific in its own class, to reuse for LSP proxies
    void readError();
    void readOutput();
    void initialize();
    void exitServer();

    void sendRequest(nlohmann::json jsonRequest);
    void initializeCallback(InitializeRequest::Response response);

private:
    std::shared_ptr<spdlog::logger> m_logger;
    const QString m_program;
    const QStringList m_arguments;
    QProcess *m_process = nullptr;

    std::unordered_map<MessageId, std::function<void(nlohmann::json)>> m_callbacks;
};

}
