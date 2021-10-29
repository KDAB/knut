#include "rc_utils.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace RcCore {

std::shared_ptr<spdlog::logger> logger()
{
    static std::shared_ptr<spdlog::logger> log = spdlog::get("rc");
    if (!log) {
        log = spdlog::stdout_color_mt("rc");
        log->set_level(spdlog::level::debug);
    }
    return log;
}

} // namespace RcCore
