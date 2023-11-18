cmake_minimum_required(VERSION 3.15)
include(FetchContent)

project(
  knut
  VERSION 1.1
  LANGUAGES CXX)

# Project initialization
# ##############################################################################
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_definitions(-DTEST_DATA_PATH="${CMAKE_SOURCE_DIR}/test_data")
add_definitions(-DKNUT_VERSION="${PROJECT_VERSION}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
include(InstallDependency)
include(CheckSubmodule)

option(USE_ASAN "use Address Sanitizer" OFF)

if(USE_ASAN)
  # /MD will be used implicitly
  add_compile_options($<$<CONFIG:Debug>:-fsanitize=address>)
  add_link_options($<$<CONFIG:Debug>:-fsanitize=address>)
endif()

# Qt
# ##############################################################################
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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
# Ensure we have the appropriate compile_commands.json files availabe.

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
  add_compile_options(-Wall -Wextra -Werror -Wno-error=comment)
endif()
# Set tests here, as it was disabled before
enable_testing()

# Knut (finally)
# ##############################################################################
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)

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

add_custom_command(
  OUTPUT scripts_copy
  COMMENT "Copying all scripts"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/bin/scripts"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/scripts"
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/scripts"
    "${CMAKE_BINARY_DIR}/bin/scripts")

add_custom_target(
  scripts ALL
  COMMENT "Copy all scripts"
  DEPENDS scripts_copy)
