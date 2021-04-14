#pragma once

#include <spdlog/spdlog.h>

#include <memory>

class LogSilencer
{
public:
    LogSilencer(const std::string &name = "")
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

private:
    spdlog::level::level_enum m_level;
    std::shared_ptr<spdlog::logger> m_logger;
};
