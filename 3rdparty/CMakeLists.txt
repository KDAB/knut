# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# spdlog
# ##############################################################################
check_submodule(spdlog spdlog)
# See https://github.com/gabime/spdlog/issues/1190
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(spdlog)

# nlohmann/json
# ##############################################################################
check_submodule(nlohmann-json nlohmann-json)
set(JSON_ImplicitConversions OFF)
add_subdirectory(nlohmann-json)

# pugixml
# ##############################################################################
check_submodule(pugixml pugixml)
add_subdirectory(pugixml)

# KDAlgorithms
# ##############################################################################
check_submodule(kdalgorithms kdalgorithms)
add_subdirectory(kdalgorithms)

# ECM
# ##############################################################################
check_submodule(extra-cmake-modules extra-cmake-modules)
# cmake-lint: disable=C0301
install_dependency(
  ECM
  "${CMAKE_CURRENT_SOURCE_DIR}/extra-cmake-modules"
  "-DBUILD_TESTING=OFF;-DBUILD_HTML_DOCS=OFF;-DBUILD_MAN_DOCS=OFF;-DBUILD_QTHELP_DOCS=OFF"
)
find_package(ECM CONFIG REQUIRED)
list(PREPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# KSyntaxHighlighting
# ##############################################################################
# A bit of a pain to integrate: - compile error for me in Qt Creator, needs to
# compile on the command line - leak 2 targets in the project Don't build tests
# and examples
set(KSYNTAXHIGHLIGHTING_USE_GUI OFF)
set(DISABLE_ALL_OPTIONAL_SUBDIRECTORIES TRUE)
set(BUILD_TESTING OFF)
check_submodule(ksyntaxhighlighting ksyntaxhighlighting)
add_subdirectory(ksyntaxhighlighting)

# TreeSitter
# ##############################################################################
check_submodule(tree-sitter tree-sitter)
project(TreeSitter LANGUAGES C)

add_library(${PROJECT_NAME} STATIC tree-sitter/lib/src/lib.c)
target_include_directories(${PROJECT_NAME} PUBLIC tree-sitter/lib/include)
# Always build tree-sitter with optimizations enabled. We shouldn't have to
# debug it and it's performance critical.
target_compile_options(${PROJECT_NAME} PRIVATE -O3)

# TreeSitterCpp
# ##############################################################################
# Currently, bumping TreeSitterCpp to 0.20.3 causes the Parser to get hung up.
# 0.20.2 is the latest version that works as of 2023-Oct-11 This may be related
# to: https://github.com/tree-sitter/tree-sitter/issues/2684
check_submodule(tree-sitter-cpp tree-sitter-cpp)
project(TreeSitterCpp LANGUAGES C)

add_library(${PROJECT_NAME} STATIC tree-sitter-cpp/src/parser.c
                                   tree-sitter-cpp/src/scanner.c)
target_link_libraries(${PROJECT_NAME} PRIVATE TreeSitter)
# Always build tree-sitter with optimizations enabled. We shouldn't have to
# debug it and it's performance critical.
target_compile_options(${PROJECT_NAME} PRIVATE -O3)
