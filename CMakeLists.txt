cmake_minimum_required(VERSION 3.15)
include(FetchContent)

project(knut VERSION 1.0 LANGUAGES CXX)

# Project initialization
###########################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_definitions(-DTEST_DATA_PATH="${CMAKE_SOURCE_DIR}/test_data")
add_definitions(-DKNUT_VERSION="${PROJECT_VERSION}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Qt
###########################################################
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Qml Quick Test UiTools REQUIRED)

# spdlog
###########################################################
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.9.2
)
# See https://github.com/gabime/spdlog/issues/1190
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_MakeAvailable(spdlog)

# nlohmann/json
###########################################################
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG        v3.10.5
)
set(JSON_ImplicitConversions OFF)
FetchContent_MakeAvailable(json)

#pugixml
###########################################################
FetchContent_Declare(pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        v1.11.4
)
FetchContent_MakeAvailable(pugixml)

#ECM
###########################################################
# Couldn't find a way to initialize it with FetchContent :(
find_package(ECM 5.91.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMEnableSanitizers)
if (ECM_ENABLE_SANITIZERS)
    add_definitions(-DSANITIZERS_ENABLED)
endif()

#KSyntaxHighlighting
###########################################################
# A bit of a pain to integrate:
#  - compile error for me in Qt Creator, needs to compile on the command line
#  - does not compile with CMAKE_CXX_STANDARD 23
#  - leak 2 targets in the project
#  - call ECM/KDEClangFormat, which inconditionnaly replace your pre-commit hook :(
option(USE_SYNTAX_HIGHLIGHTING "Enable syntax highlighting in IDE" ON)
if(USE_SYNTAX_HIGHLIGHTING)
    FetchContent_Declare(KSyntaxHighlighting
        GIT_REPOSITORY https://github.com/narnaud/syntax-highlighting.git
        GIT_TAG        b8dce05d42aa7a8b8baea4f708d5278961a693fd
    )
    # Don't build tests and examples
    set(KSYNTAXHIGHLIGHTING_USE_GUI OFF)
    set(DISABLE_ALL_OPTIONAL_SUBDIRECTORIES TRUE)
    set(BUILD_TESTING OFF)
    FetchContent_MakeAvailable(KSyntaxHighlighting)
    add_definitions(-DUSE_SYNTAX_HIGHLIGHTING)
endif()

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
add_subdirectory(scripts)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)

file(GLOB DOCS_FILES README.md mkdocs.yml docs/*.md docs/contributing/*.md docs/getting-started/*.md)
add_custom_target(docs SOURCES ${DOCS_FILES})
