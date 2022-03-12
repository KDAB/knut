#include "logger.h"

#include <spdlog/spdlog.h>

namespace Core {

LoggerObject::LoggerObject()
    : m_firstLogger(!m_isLogging)
{
}

LoggerObject::~LoggerObject()
{
    if (m_firstLogger)
        m_isLogging = false;
}

void LoggerObject::log(const QString &string)
{
    if (m_isLogging)
        return;
    spdlog::trace(string.toStdString());
    m_isLogging = true;
}

} // namespace Core
