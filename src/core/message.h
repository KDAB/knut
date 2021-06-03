#pragma once

#include <QObject>

#include <spdlog/spdlog.h>

namespace Core {

class Message : public QObject
{
    Q_OBJECT

public:
    explicit Message(QObject *parent = nullptr);
    ~Message() override;

public slots:
    void error(const QString &text);
    void log(const QString &text);
    void debug(const QString &text);
    void warning(const QString &text);

private:
    std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace Core
