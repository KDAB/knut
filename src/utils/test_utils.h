#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QString>

#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

namespace Test {

inline QString testDataPath()
{
    QString path;
#if defined(TEST_DATA_PATH)
    path = TEST_DATA_PATH;
#endif
    if (path.isEmpty() || !QDir(path).exists()) {
        path = QCoreApplication::applicationDirPath() + "/test_data";
    }
    return path;
}

class LogSilencer
{
public:
    inline static std::string Default = "";

    LogSilencer(const std::string &name = Default)
    {
        if (name.empty())
            m_logger = spdlog::default_logger();
        else
            m_logger = spdlog::get(name);
        if (m_logger) {
            m_level = m_logger->level();
            m_logger->set_level(spdlog::level::off);
        }
    }
    ~LogSilencer()
    {
        if (m_logger) {
            m_logger->set_level(m_level);
        }
    }

    LogSilencer(LogSilencer &&) noexcept = default;
    LogSilencer &operator=(LogSilencer &&) noexcept = default;

private:
    spdlog::level::level_enum m_level = spdlog::level::off;
    std::shared_ptr<spdlog::logger> m_logger;
};

class LogSilencers
{
public:
    LogSilencers(std::initializer_list<std::string> names)
    {
        m_logs.reserve(names.size());
        for (const auto &name : names)
            m_logs.emplace_back(name);
    }

private:
    std::vector<LogSilencer> m_logs;
};
}
