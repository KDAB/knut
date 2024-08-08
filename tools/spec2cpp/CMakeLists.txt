# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(
  spec2cpp
  VERSION 1
  LANGUAGES CXX)

add_definitions(-DLSP_SOURCE_PATH="${CMAKE_SOURCE_DIR}/src/lsp")

set(PROJECT_SOURCES data.h spec2cpp.cpp specparser.h specparser.cpp
                    specwriter.h specwriter.cpp)

qt_add_resources(PROJECT_SOURCES spec2cpp.qrc)

add_executable(${PROJECT_NAME} EXCLUDE_FROM_ALL ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json
                                              Qt::Core)
