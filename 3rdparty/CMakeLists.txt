# spdlog
# ##########################################################
CheckSubmodule(spdlog spdlog)
# See https://github.com/gabime/spdlog/issues/1190
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(SPDLOG_USE_STD_FORMAT ON)
add_subdirectory(spdlog)

# nlohmann/json
# ##########################################################
CheckSubmodule(nlohmann-json nlohmann-json)
set(JSON_ImplicitConversions OFF)
add_subdirectory(nlohmann-json)

# pugixml
# ##########################################################
CheckSubmodule(pugixml pugixml)
add_subdirectory(pugixml)

# KDAlgorithms
# ##########################################################
CheckSubmodule(kdalgorithms kdalgorithms)
add_subdirectory(kdalgorithms)

# ECM
# ##########################################################
CheckSubmodule(extra-cmake-modules extra-cmake-modules)
InstallDependency(ECM "${CMAKE_CURRENT_SOURCE_DIR}/extra-cmake-modules" "-DBUILD_TESTING=OFF;-DBUILD_HTML_DOCS=OFF;-DBUILD_MAN_DOCS=OFF;-DBUILD_QTHELP_DOCS=OFF")
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
    CheckSubmodule(ksyntaxhighlighting ksyntaxhighlighting)
    add_subdirectory(ksyntaxhighlighting)
    add_definitions(-DUSE_SYNTAX_HIGHLIGHTING)
endif()

# TreeSitter
###########################################################
CheckSubmodule(tree-sitter tree-sitter)
project(TreeSitter LANGUAGES C)

add_library(${PROJECT_NAME} STATIC
tree-sitter/lib/src/lib.c)
target_include_directories(${PROJECT_NAME}
  PUBLIC tree-sitter/lib/include)


# TreeSitterCpp
###########################################################
# Currently, bumping TreeSitterCpp to 0.20.3 causes the Parser
# to get hung up. 0.20.2 is the latest version that works as of 2023-Oct-11
# This may be related to: https://github.com/tree-sitter/tree-sitter/issues/2684
CheckSubmodule(tree-sitter-cpp tree-sitter-cpp)
project(TreeSitterCpp LANGUAGES C)

add_library(${PROJECT_NAME} STATIC
  tree-sitter-cpp/src/parser.c
  tree-sitter-cpp/src/scanner.c)
target_link_libraries(${PROJECT_NAME} PRIVATE TreeSitter)
