#pragma once

#include <spdlog/spdlog.h>

#if defined(SPDLOG_USE_STD_FORMAT)
#include "qt_std_format.h"
#else
#include "qt_fmt_format.h"
#endif
