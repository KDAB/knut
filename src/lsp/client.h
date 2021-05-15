#pragma once

#include "requests.h"

#include <QObject>

#include <spdlog/spdlog.h>

#include <string>

namespace Lsp {

class ClientBackend;

class Client : public QObject
{
    Q_OBJECT

public:
    enum State {
        Uninitialized,
        Initialized,
        Shutdown,
        Error,
    };

    explicit Client(const std::string &language, QString program, QStringList arguments, QObject *parent = nullptr);
    ~Client();

    bool initialize();
    bool shutdown();

    State state() const { return m_state; }

signals:
    void stateChanged(State state);

private:
    void setState(State newState);
    bool initializeCallback(InitializeRequest::Response response);
    bool shutdownCallback(ShutdownRequest::Response response);

private:
    mutable int m_nextRequestId = 1;
    std::shared_ptr<spdlog::logger> m_clientLogger;
    ClientBackend *m_backend = nullptr;
    State m_state = Uninitialized;
};

} // namespace Lsp
