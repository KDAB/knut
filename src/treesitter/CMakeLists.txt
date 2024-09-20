# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(knut-treesitter LANGUAGES CXX)

set(PROJECT_SOURCES
    node.cpp
    parser.cpp
    predicates.cpp
    query.cpp
    tree.cpp
    tree_cursor.cpp
    node.h
    parser.h
    predicates.h
    query.h
    tree.h
    tree_cursor.h)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(
  ${PROJECT_NAME}
  TreeSitter
  TreeSitterCpp
  TreeSitterQmlJs
  kdalgorithms
  knut-utils
  Qt::Core)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
