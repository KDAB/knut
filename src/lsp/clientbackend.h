/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "requestmessage.h"
#include "utils/json.h"
#include "utils/log.h"

#include <QObject>
#include <QProcess>
#include <functional>
#include <unordered_map>

class QProcess;

namespace Lsp {

class ClientBackend : public QObject
{
    Q_OBJECT

public:
    ClientBackend(const std::string &language, QString program, QStringList arguments, QObject *parent = nullptr);
    ~ClientBackend() override;

    bool start();

    template <typename Request>
    void sendAsyncRequest(const Request &request, typename Request::ResponseCallback callback)
    {
        m_callbacks[request.id] = [this, callback](nlohmann::json &&j) {
            if (callback) {
                auto response = deserializeResponse<typename Request::Response>(std::move(j));
                callback(std::move(response));
            }
        };
        sendAsyncJsonRequest(request);
    }

    template <typename Request>
    typename Request::Response sendRequest(const Request &request)
    {
        m_callbacks[request.id] = [this](nlohmann::json &&j) {
            m_response = std::move(j);
            emit responseEmitted(QPrivateSignal {});
        };

        std::visit(
            [this, &request](const auto &id) {
                if (m_serverLogger)
                    m_serverLogger->debug("==> Sending Request {} with id {}", request.method, id);
            },
            request.id);
        auto j = sendJsonRequest(request);
        return deserializeResponse<typename Request::Response>(std::move(j));
    }

    template <typename Notification>
    void sendNotification(const Notification &notification)
    {
        if (m_serverLogger)
            m_serverLogger->debug("==> Sending Notification {}", notification.method);
        sendJsonNotification(notification);
    }

signals:
    void errorOccured(const QString &message);
    void finished();
    void responseEmitted(QPrivateSignal);

private:
    void readError();
    void readOutput();
    void handleError();
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus);

    template <typename Response>
    Response deserializeResponse(nlohmann::json &&j)
    {
        try {
            auto response = j.get<Response>();
            std::visit(
                [this](const auto &id) {
                    if (m_serverLogger)
                        m_serverLogger->error("<== Receiving Response id {}", id);
                },
                response.id);
            return response;
        } catch (...) {
            if (m_serverLogger)
                m_serverLogger->error("<== Invalid response from server: {}", j.dump());
        }
        return {};
    }

    void sendAsyncJsonRequest(const nlohmann::json &jsonRequest);
    nlohmann::json sendJsonRequest(const nlohmann::json &jsonRequest);
    void sendJsonNotification(const nlohmann::json &jsonNotification);

    void logMessage(std::string type, const nlohmann::json &message);

private:
    std::shared_ptr<spdlog::logger> m_serverLogger;
    std::shared_ptr<spdlog::logger> m_messageLogger;
    const QString m_program;
    const QStringList m_arguments;
    QProcess *m_process = nullptr;

    std::unordered_map<MessageId, std::function<void(nlohmann::json)>> m_callbacks;
    nlohmann::json m_response;

    class Message
    {
    public:
        void addData(const QByteArray &data);

        // Parse the current data, and return a message as a json object or empty if there's nothing
        nlohmann::json getNextMessage();

    private:
        // Read the header file and remove the data from m_data
        // Returns true if the header file is read
        bool readHeader();

    private:
        QByteArray m_data;
        int m_length = 0;
    };

    Message m_message;
};

}
