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

set(PROJECT_SOURCES
    json.h
    qtuiwriter.h
    qtuiwriter.cpp
    qt_fmt_helpers.h
    qt_fmt_format.h
    string_helper.h
    string_helper.cpp
    log.h)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC nlohmann_json::nlohmann_json spdlog::spdlog
         Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui
         pugixml::pugixml)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
