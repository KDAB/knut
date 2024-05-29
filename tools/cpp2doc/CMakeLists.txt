# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(
  cpp2doc
  VERSION 1
  LANGUAGES CXX)

add_definitions(-DKNUT_SOURCE_PATH="${CMAKE_SOURCE_DIR}/src")
add_definitions(-DKNUT_DOC_PATH="${CMAKE_SOURCE_DIR}/docs")
add_definitions(-DKNUT_MKDOCS_PATH="${CMAKE_SOURCE_DIR}/mkdocs.yml")

set(PROJECT_SOURCES data.h cpp2doc.cpp sourceparser.h sourceparser.cpp
                    docwriter.h docwriter.cpp)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core)
