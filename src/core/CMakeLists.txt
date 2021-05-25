project(knut-core LANGUAGES CXX)

set(PROJECT_SOURCES
    dir.cpp
    dir.h
    file.cpp
    file.h
    fileinfo.cpp
    fileinfo.h
    json_utils.h
    qdirvaluetype.cpp
    qdirvaluetype.h
    qfileinfovaluetype.cpp
    qfileinfovaluetype.h
    scriptitem.cpp
    scriptitem.h
    settings.cpp
    settings.h
    string_utils.cpp
    string_utils.h
)

qt5_add_resources(PROJECT_QRC
    core.qrc
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_QRC})
target_link_libraries(${PROJECT_NAME} PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Qml
    knut-lsp
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
