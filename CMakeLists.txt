#
# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

cmake_minimum_required(VERSION 3.15)
# This enables the use of CMAKE_MSVC_DEBUG_INFORMATION_FORMAT as required by the
# CI preset
cmake_policy(SET CMP0141 NEW)

include(FetchContent)

# file(READ ...) would insert the trailing newline character at the end of the
# file into the version, so use file(STRINGS ...) instead.
file(STRINGS version.txt KNUT_VERSION_FILE)
list(GET KNUT_VERSION_FILE 0 KNUT_VERSION)

project(
  knut
  VERSION "${KNUT_VERSION}"
  LANGUAGES CXX)
message(STATUS "Knut version: ${PROJECT_VERSION}")

# Project initialization
# ##############################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_definitions(-DEXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")
add_definitions(-DTEST_DATA_PATH="${CMAKE_SOURCE_DIR}/test_data")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
include(InstallDependency)
include(CheckSubmodule)

option(USE_ASAN "use Address Sanitizer" OFF)
# It's best practice to only enable -Werror in CI & development builds. We
# enable this option in the appropriate CMakePresets. If you just want to have a
# working build of knut, -Werror can be very annoying, so keep it off by
# default.
option(KNUT_ERROR_ON_WARN
       "Issue a compiler error if the compiler encounters a warning" OFF)
option(
  KNUT_UPDATE_DOCS
  "Automatically update the documentation markdown files on each build.\n \
  If cpp2doc cannot be executed on your setup, consider disabling this feature."
  ON)

if(USE_ASAN)
  # /MD will be used implicitly
  add_compile_options($<$<CONFIG:Debug>:-fsanitize=address>)
  add_link_options($<$<CONFIG:Debug>:-fsanitize=address>)
endif()

# Project version
# ##############################################################################
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

# Generate build date for the about dialog Note: This is the simplest way to get
# a build date out of CMake Drawbacks: The timestamp is only updated when CMake
# is rerun. But this should sufficient for our use-case.
string(TIMESTAMP KNUT_BUILDDATE "%Y-%m-%d")

# Qt
# ##############################################################################
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6)
find_package(
  Qt6
  COMPONENTS Widgets Qml Quick Test UiTools
  REQUIRED)

# 3rdparty
# ##############################################################################
add_subdirectory(3rdparty)
add_subdirectory(3rdparty-kdab)

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
if(KNUT_ERROR_ON_WARN)
  if(MSVC)
    add_compile_options(/W3 /WX)
  else()
    add_compile_options(-Wall -Wextra -Werror -Wno-comment -Wno-error=comment)
  endif()
endif()
# Set tests here, as it was disabled before
enable_testing()

# Knut (finally)
# ##############################################################################
add_subdirectory(src)
add_subdirectory(tools)

set(KNUT_BINARY_PATH "$<TARGET_FILE:knut>")
add_subdirectory(tests)

# mfc-utils and photonwidgets are private to KDAB, so make them optional so that
# non-KDABians can still develop Knut.
if(EXISTS ${CMAKE_SOURCE_DIR}/3rdparty-kdab/mfc-utils/tests-scripts)
  add_subdirectory(3rdparty-kdab/mfc-utils/tests-scripts)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/3rdparty-kdab/photonwidgets/tests/scripts)
  add_subdirectory(3rdparty-kdab/photonwidgets/tests/scripts)
endif()

file(
  GLOB
  DOCS_FILES
  README.md
  mkdocs.yml
  docs/*.md
  docs/contributing/*.md
  docs/getting-started/*.md)

if(KNUT_UPDATE_DOCS)
  add_custom_target(
    docs ALL
    cpp2doc
    COMMENT "Update documentation"
    SOURCES ${DOCS_FILES})
else()
  add_custom_target(
    docs
    cpp2doc
    COMMENT "Update documentation"
    SOURCES ${DOCS_FILES})
endif()

if(WIN32)
    configure_file(AppxManifest.xml.in AppxManifest.xml @ONLY)
endif()
