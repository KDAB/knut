project(knut-ui LANGUAGES CXX)

set(PROJECT_SOURCES
    knutmain.h
    knutmain.cpp
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_QRC})
target_link_libraries(${PROJECT_NAME} PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    Qt${QT_VERSION_MAJOR}::Core
    knut-core
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
