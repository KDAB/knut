cmake_minimum_required(VERSION 3.15)
include(FetchContent)

project(knut VERSION 4.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_definitions(-DTEST_DATA_PATH="${CMAKE_SOURCE_DIR}/test_data")
add_definitions(-DKNUT_VERSION="${PROJECT_VERSION}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(ECM 5.91.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Qml Quick Test UiTools REQUIRED)

# spdlog
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.9.2
)
# See https://github.com/gabime/spdlog/issues/1190
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# nlohmann/json
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG        v3.10.5
)
set(JSON_ImplicitConversions OFF)

#pugixml
FetchContent_Declare(pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        v1.11.4
)

# Fetch them all
FetchContent_MakeAvailable(spdlog json pugixml)

option(USE_SYNTAX_HIGHLIGHTING "Enable syntax highlighting in IDE" ON)
if(USE_SYNTAX_HIGHLIGHTING)
    #KSyntaxHighlighting
    FetchContent_Declare(KSyntaxHighlighting
        GIT_REPOSITORY https://github.com/KDE/syntax-highlighting.git
        GIT_TAG        v5.91.0
    )
    # Don't build tests and examples
    set(KSYNTAXHIGHLIGHTING_USE_GUI OFF)
    set(DISABLE_ALL_OPTIONAL_SUBDIRECTORIES TRUE)
    set(BUILD_TESTING OFF)
    FetchContent_MakeAvailable(KSyntaxHighlighting)
    add_definitions(-DUSE_SYNTAX_HIGHLIGHTING)
endif()

if(MSVC)
    add_compile_options(/W3 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -Wno-error=comment)
endif()

enable_testing()

# ECM
include(ECMEnableSanitizers)
if (ECM_ENABLE_SANITIZERS)
    add_definitions(-DSANITIZERS_ENABLED)
endif()

# Knut
add_subdirectory(scripts)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)
add_subdirectory(test_data)

file(GLOB DOCS_FILES README.md mkdocs.yml docs/*.md docs/contributing/*.md)
add_custom_target(docs SOURCES ${DOCS_FILES})
