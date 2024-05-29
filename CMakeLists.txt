cmake_minimum_required(VERSION 3.15)
# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

include(FetchContent)

project(
  knut
  VERSION 1.0
  LANGUAGES CXX)

# Project initialization
# ##############################################################################
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_definitions(-DTEST_DATA_PATH="${CMAKE_SOURCE_DIR}/test_data")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
include(InstallDependency)
include(CheckSubmodule)

option(USE_ASAN "use Address Sanitizer" OFF)
option(KNUT_USE_STD_FORMAT "Use std format library for speed log library" ON)

if(USE_ASAN)
  # /MD will be used implicitly
  add_compile_options($<$<CONFIG:Debug>:-fsanitize=address>)
  add_link_options($<$<CONFIG:Debug>:-fsanitize=address>)
endif()

# Project version
# ##############################################################################
add_definitions(-DKNUT_VERSION="${PROJECT_VERSION}")
set(KNUT_VERSION_STRING "${PROJECT_VERSION}")

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  find_package(Git)
  if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_REVISION)
    string(REGEX REPLACE "\n" "" GIT_REVISION "${GIT_REVISION}")
    message(STATUS "Git revision ${GIT_REVISION} with exe ${GIT_EXECUTABLE}")
    set(KNUT_VERSION_STRING
        "${KNUT_VERSION_STRING} (revision: ${GIT_REVISION})")
  else()
    message(STATUS "GIT not found!?")
  endif()
endif()
add_definitions(-DKNUT_VERSION_STRING="${KNUT_VERSION_STRING}")

# Generate build date for the about dialog Note: This is the simplest way to get
# a build date out of CMake Drawbacks: The timestamp is only updated when CMake
# is rerun. But this should sufficient for our use-case.
string(TIMESTAMP KNUT_BUILDDATE "%Y-%m-%d")
add_definitions(-DKNUT_BUILDDATE="${KNUT_BUILDDATE}")

# Qt
# ##############################################################################
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5)
find_package(
  Qt6
  COMPONENTS Widgets Qml Quick Test UiTools
  REQUIRED)

# 3rdparty
# ##############################################################################
add_subdirectory(3rdparty)

# test_data
# ##############################################################################
# Important for tests - generates the appropriate compile_commands.json for the
# test_data. Needs to be done before the setting "-Werror".
add_subdirectory(test_data)

# compile_commands.json
# ##############################################################################
# Ensure we have the appropriate compile_commands.json files available.

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# cmake-lint: disable=C0301
add_custom_target(
  symlink-compile-commands ALL
  COMMENT "Create compile_commands.json files"
  DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json
    ${CMAKE_CURRENT_LIST_DIR}/test_data/projects/cpp-project/compile_commands.json
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json
  COMMENT "Creating compile_commands.json file for Knut"
  COMMAND ${CMAKE_COMMAND} -E create_symlink
          ${CMAKE_BINARY_DIR}/compile_commands.json compile_commands.json
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_LIST_DIR}/test_data/projects/cpp-project/compile_commands.json
  COMMENT "Creating compile_commands.json file for cpp-project"
  COMMAND ${CMAKE_COMMAND} -E create_symlink
          ${CMAKE_BINARY_DIR}/compile_commands.json compile_commands.json
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test_data/projects/cpp-project)

# Post-initialization
# ##############################################################################
# Need to be done *after* compiling 3rd parties
if(MSVC)
  add_compile_options(/W3 /WX)
else()
  add_compile_options(-Wall -Wextra -Werror -Wno-comment -Wno-error=comment)
endif()
# Set tests here, as it was disabled before
enable_testing()

# Knut (finally)
# ##############################################################################
add_subdirectory(src)
add_subdirectory(tools)

set(KNUT_BINARY_PATH "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/knut")
add_subdirectory(tests)
add_subdirectory(3rdparty/mfc-utils/tests-scripts)
add_subdirectory(3rdparty/photonwidgets/tests/scripts)

file(
  GLOB
  DOCS_FILES
  README.md
  mkdocs.yml
  docs/*.md
  docs/contributing/*.md
  docs/getting-started/*.md)
add_custom_target(
  docs ALL
  cpp2doc
  COMMENT "Update documentation"
  SOURCES ${DOCS_FILES})
