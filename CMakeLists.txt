cmake_minimum_required(VERSION 3.11)
include(FetchContent)

project(knut LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
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
    GIT_TAG v3.9.1
)
set(JSON_ImplicitConversions OFF)

# doctest
FetchContent_Declare(doctest
    GIT_REPOSITORY "https://github.com/onqtam/doctest"
    GIT_TAG 2.4.6
)


# magic_enum
FetchContent_Declare(magic_enum
    GIT_REPOSITORY "https://github.com/Neargye/magic_enum"
    GIT_TAG v0.7.2
)

FetchContent_MakeAvailable(spdlog json doctest magic_enum)

# Knut
add_subdirectory(src)
