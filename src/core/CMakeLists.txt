project(knut-core LANGUAGES CXX)

set(PROJECT_SOURCES
    dir.h
    dir.cpp
    document.h
    document.cpp
    file.h
    file.cpp
    fileinfo.h
    fileinfo.cpp
    json_utils.h
    knutmain.h
    knutmain.cpp
    message.h
    message.cpp
    qdirvaluetype.h
    qdirvaluetype.cpp
    qfileinfovaluetype.h
    qfileinfovaluetype.cpp
    scriptitem.h
    scriptitem.cpp
    scriptmanager.h
    scriptmanager.cpp
    scriptrunner.h
    scriptrunner.cpp
    settings.h
    settings.cpp
    string_utils.h
    string_utils.cpp
    testutil.h
    testutil.cpp
    utils.h
    utils.cpp
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
    Qt${QT_VERSION_MAJOR}::QmlPrivate
    Qt${QT_VERSION_MAJOR}::Quick
    knut-lsp
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
