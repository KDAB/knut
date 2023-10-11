cmake_minimum_required(VERSION 3.15)
include(FetchContent)

project(knut VERSION 1.1 LANGUAGES CXX)

# Project initialization
###########################################################
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

# Qt
###########################################################
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Qml Quick Test UiTools REQUIRED)

# spdlog
# ##########################################################
CheckSubmodule(spdlog 3rdparty/spdlog)
# See https://github.com/gabime/spdlog/issues/1190
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(3rdparty/spdlog)

# nlohmann/json
# ##########################################################
CheckSubmodule(nlohmann-json 3rdparty/nlohmann-json)
set(JSON_ImplicitConversions OFF)
add_subdirectory(3rdparty/nlohmann-json)

# pugixml
# ##########################################################
CheckSubmodule(pugixml 3rdparty/pugixml)
add_subdirectory(3rdparty/pugixml)

# KDAlgorithms
# ##########################################################
CheckSubmodule(kdalgorithms 3rdparty/kdalgorithms)
add_subdirectory(3rdparty/kdalgorithms)

# ECM
# ##########################################################
CheckSubmodule(extra-cmake-modules 3rdparty/extra-cmake-modules)
InstallDependency(ECM "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/extra-cmake-modules" "-DBUILD_TESTING=OFF;-DBUILD_HTML_DOCS=OFF;-DBUILD_MAN_DOCS=OFF;-DBUILD_QTHELP_DOCS=OFF")
find_package(ECM CONFIG REQUIRED)
list(PREPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMEnableSanitizers)
if (ECM_ENABLE_SANITIZERS)
    add_definitions(-DSANITIZERS_ENABLED)
endif()

#KSyntaxHighlighting
###########################################################
# A bit of a pain to integrate:
#  - compile error for me in Qt Creator, needs to compile on the command line
#  - leak 2 targets in the project
option(USE_SYNTAX_HIGHLIGHTING "Enable syntax highlighting in IDE" ON)
if(USE_SYNTAX_HIGHLIGHTING)
    # Don't build tests and examples
    set(KSYNTAXHIGHLIGHTING_USE_GUI OFF)
    set(DISABLE_ALL_OPTIONAL_SUBDIRECTORIES TRUE)
    set(BUILD_TESTING OFF)
    CheckSubmodule(ksyntaxhighlighting 3rdparty/ksyntaxhighlighting)
    add_subdirectory(3rdparty/ksyntaxhighlighting)
    add_definitions(-DUSE_SYNTAX_HIGHLIGHTING)
endif()

#TreeSitter and TreeSitterCpp
###########################################################
CheckSubmodule(tree-sitter 3rdparty/tree-sitter)
# Currently, bumping TreeSitterCpp to 0.20.3 causes the Parser
# to get hung up. 0.20.2 is the latest version that works as of 2023-Oct-11
# This may be related to: https://github.com/tree-sitter/tree-sitter/issues/2684
CheckSubmodule(tree-sitter-cpp 3rdparty/tree-sitter-cpp)
add_subdirectory(3rdparty)

# test_data
###########################################################
# Important for tests - generates the appropriate compile_commands.json for the test_data.
# Needs to be done before the setting "-Werror".
add_subdirectory(test_data)

# compile_commands.json
###########################################################
# Ensure we have the appropriate compile_commands.json files availabe.

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_custom_target(symlink-compile-commands ALL
    DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json
        ${CMAKE_CURRENT_LIST_DIR}/test_data/projects/cpp-project/compile_commands.json
    )

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/compile_commands.json
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json compile_commands.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/test_data/projects/cpp-project/compile_commands.json
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json compile_commands.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test_data/projects/cpp-project
    )

# Post-initialization
###########################################################
# Need to be done *after* compiling 3rd parties
if(MSVC)
    add_compile_options(/W3 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -Wno-error=comment)
endif()
#Set tests here, as it was disabled before
enable_testing()

# Knut (finally)
###########################################################
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)

file(GLOB DOCS_FILES README.md mkdocs.yml docs/*.md docs/contributing/*.md docs/getting-started/*.md)
add_custom_target(docs ALL cpp2doc SOURCES ${DOCS_FILES})

add_custom_command(OUTPUT scripts_copy
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/bin/scripts"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/scripts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/scripts" "${CMAKE_BINARY_DIR}/bin/scripts"
)
add_custom_target(scripts ALL DEPENDS scripts_copy)
