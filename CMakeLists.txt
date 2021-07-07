cmake_minimum_required(VERSION 3.11)
include(FetchContent)

project(knut VERSION 4.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
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

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Qml Quick Test UiTools REQUIRED)

# spdlog
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.8.5
)
# See https://github.com/gabime/spdlog/issues/1190
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_MakeAvailable(spdlog)

# nlohmann/json
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG        v3.9.1
)
set(JSON_ImplicitConversions OFF)
FetchContent_MakeAvailable(json)

#pugixml
FetchContent_Declare(pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        v1.11.4
)
FetchContent_MakeAvailable(pugixml)

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
