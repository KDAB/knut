project(knut VERSION 4.0 LANGUAGES CXX)

set(PROJECT_SOURCES
	main.cpp
        core/scriptitem.cpp
        core/scriptitem.h
        core/settings.cpp
        core/settings.h
        lsp/client.cpp
        lsp/client.h
        lsp/lsp_types.h
        lsp/requestmessage.h
        lsp/initialize_request.h
        lsp/shutdown_request.h
        lsp/notificationmessage.h
        script/scriptmanager.cpp
        script/scriptmanager.h
        script/scriptrunner.cpp
        script/scriptrunner.h
        utils/json_utils.h
        utils/log_utils.h
)

qt5_add_resources(PROJECT_QRC
        core/core.qrc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_QRC})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::QmlPrivate
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    doctest::doctest
    magic_enum::magic_enum
)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif()
