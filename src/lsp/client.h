#pragma once

#include <QObject>

#include <spdlog/spdlog.h>

class QProcess;

namespace Lsp {

class Client : public QObject
{
    Q_OBJECT

public:
    Client(const QString &program, const QStringList &arguments, QObject *parent = nullptr);

    void start();

private:
    void readError();
    void readOutput();
    void finishProcess();

private:
    std::shared_ptr<spdlog::logger> m_logger;
    const QString m_program;
    const QStringList m_arguments;
    QProcess *m_process = nullptr;
};

}
