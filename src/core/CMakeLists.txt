project(knut-core LANGUAGES CXX)

set(PROJECT_SOURCES
    cppdocument.h
    cppdocument.cpp
    dir.h
    dir.cpp
    document.h
    document.cpp
    file.h
    file.cpp
    fileinfo.h
    fileinfo.cpp
    json_utils.h
    knutcore.h
    knutcore.cpp
    mark.h
    mark.cpp
    message.h
    message.cpp
    project.h
    project.cpp
    qdirvaluetype.h
    qdirvaluetype.cpp
    qfileinfovaluetype.h
    qfileinfovaluetype.cpp
    rcdocument.h
    rcdocument.cpp
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
    textdocument.h
    textdocument.cpp
    uidocument.h
    uidocument.cpp
    utils.h
    utils.cpp
)

qt_add_resources(PROJECT_SOURCES
    core.qrc
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    pugixml::pugixml
    Qt::Core
    Qt::Qml
    Qt::QmlPrivate
    Qt::Quick
    Qt::Widgets
    Qt::UiTools
    knut-lsp
    knut-rccore
    knut-rcui
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
