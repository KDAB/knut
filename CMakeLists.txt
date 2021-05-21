cmake_minimum_required(VERSION 3.11)
include(FetchContent)

project(knut VERSION 4.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_definitions(-DTEST_DATA_PATH="${CMAKE_SOURCE_DIR}/test_data")
add_definitions(-DKNUT_VERSION="${PROJECT_VERSION}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(MSVC)
    add_compile_options(/W3 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -Wno-error=comment)
endif()

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Qml Quick Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Qml Quick Test REQUIRED)

# spdlog
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.8.5
)

# See https://github.com/gabime/spdlog/issues/1190
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# nlohmann/json
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG        v3.9.1
)
set(JSON_ImplicitConversions OFF)

# doctest
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest
    GIT_TAG        2.4.6
)

# cxxopts
FetchContent_Declare(cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts
    GIT_TAG        v2.2.1
)
if(NOT cxxopts_POPULATED)
  FetchContent_Populate(cxxopts)
  set(CXXOPTS_BUILD_EXAMPLES OFF CACHE INTERNAL "No examples for cxxopts")
  set(CXXOPTS_BUILD_TESTS OFF CACHE INTERNAL "No tests for cxxopts")
  set(CXXOPTS_ENABLE_INSTALL OFF CACHE INTERNAL "No installs for cxxopts")
  add_subdirectory(${cxxopts_SOURCE_DIR} ${cxxopts_BINARY_DIR})
endif()

FetchContent_MakeAvailable(spdlog json doctest)

# Knut
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)
add_subdirectory(test_data)
