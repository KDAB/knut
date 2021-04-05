project(knut VERSION 4.0 LANGUAGES CXX)

set(PROJECT_SOURCES
    main.cpp
    core/languages.h
    lsp/client.cpp
    lsp/client.h
    lsp/lsp_types.h
    lsp/requestmessage.h
    lsp/initialize_request.cpp
    lsp/initialize_request.h
    lsp/shutdown_request.h
    lsp/notificationmessage.h
    utils/json_utils.h
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets nlohmann_json::nlohmann_json spdlog::spdlog)
