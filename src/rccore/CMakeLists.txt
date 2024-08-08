# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(knut-rccore LANGUAGES CXX)

set(PROJECT_SOURCES
    data.h
    data.cpp
    lexer.h
    lexer.cpp
    rcfile.h
    rc_convert.cpp
    rc_parse.cpp
    rc_utility.cpp
    rc_write.cpp
    ribbon.h
    ribbon.cpp
    stream.h
    stream.cpp)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} kdalgorithms Qt::Core Qt::Gui knut-utils
                      pugixml::pugixml)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
