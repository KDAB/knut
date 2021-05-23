project(knut-interface LANGUAGES CXX)

set(PROJECT_SOURCES
    knutmain.h
    knutmain.cpp
    scriptmanager.cpp
    scriptmanager.h
    scriptrunner.cpp
    scriptrunner.h
    testutil.cpp
    testutil.h
)

qt5_add_resources(PROJECT_QRC
    interface.qrc
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES} ${PROJECT_QRC})
target_link_libraries(${PROJECT_NAME} PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::QmlPrivate
    Qt${QT_VERSION_MAJOR}::Quick
    knut-core
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
