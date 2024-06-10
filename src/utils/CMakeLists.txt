# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(knut-utils LANGUAGES CXX)
find_package(Qt6 REQUIRED COMPONENTS Core Gui)
set(PROJECT_SOURCES json.h qt_fmt_helpers.h qt_fmt_format.h strings.h
                    strings.cpp log.h)
add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json
                                             spdlog::spdlog Qt6::Core Qt6::Gui)

target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
