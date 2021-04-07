project(knut VERSION 4.0 LANGUAGES CXX)

set(PROJECT_SOURCES
	main.cpp
        core/scriptitem.cpp
        core/scriptitem.h
        lsp/client.cpp
	lsp/client.h
        lsp/lsp_types.h
        lsp/requestmessage.h
        lsp/initialize_request.cpp
        lsp/initialize_request.h
        lsp/shutdown_request.h
        lsp/notificationmessage.h
        script/scriptmanager.cpp
        script/scriptmanager.h
        script/scriptrunner.cpp
        script/scriptrunner.h
        utils/json_utils.h
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::QmlPrivate
    Qt${QT_VERSION_MAJOR}::Quick
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)
