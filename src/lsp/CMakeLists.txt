# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(knut-lsp LANGUAGES CXX)

set(PROJECT_SOURCES
    client.h
    client.cpp
    clientbackend.h
    clientbackend.cpp
    lsp_utils.h
    lsp_utils.cpp
    notificationmessage.h
    notificationmessage_json.h
    notifications.h
    requestmessage.h
    requestmessage_json.h
    requests.h
    types.h
    types_json.h
    types_json.cpp)

if(MSVC)
  add_compile_options(/bigobj)
endif()

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} knut-utils nlohmann_json::nlohmann_json
                      Qt${QT_VERSION_MAJOR}::Core)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
