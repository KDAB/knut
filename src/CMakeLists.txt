project(knut VERSION 4.0 LANGUAGES CXX)

set(PROJECT_SOURCES
	main.cpp
        core/scriptitem.cpp
        core/scriptitem.h
        core/settings.cpp
        core/settings.h
        lsp/client.cpp
        lsp/client.h
        lsp/requestmessage.h
        lsp/types.h
        lsp/initialize_request.h
        lsp/shutdown_request.h
        lsp/notificationmessage.h
        script/scriptmanager.cpp
        script/scriptmanager.h
        script/scriptrunner.cpp
        script/scriptrunner.h
        utils/json_utils.h
        utils/json_utils.cpp
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

FetchContent_GetProperties(json SOURCE_DIR JSON_SRC_DIR)
FetchContent_GetProperties(magic_enum SOURCE_DIR MAGIC_ENUM_SRC_DIR)
FetchContent_GetProperties(spdlog SOURCE_DIR SPDLOG_SRC_DIR)

# Set manually as the *-inl.h files cannot be included together with the non *-inl.h ones
set(GLOB SPDLOG_HEADERS
  ${SPDLOG_SRC_DIR}/include/spdlog/async.h
  ${SPDLOG_SRC_DIR}/include/spdlog/async-logger.h
  ${SPDLOG_SRC_DIR}/include/spdlog/common.h
  ${SPDLOG_SRC_DIR}/include/spdlog/formatter.h
  ${SPDLOG_SRC_DIR}/include/spdlog/fwd.h
  ${SPDLOG_SRC_DIR}/include/spdlog/logger.h
  ${SPDLOG_SRC_DIR}/include/spdlog/pattern_formatter.h
  ${SPDLOG_SRC_DIR}/include/spdlog/spdlog.h
  ${SPDLOG_SRC_DIR}/include/spdlog/stopwatch.h
  ${SPDLOG_SRC_DIR}/include/spdlog/tweakme.h
  ${SPDLOG_SRC_DIR}/include/spdlog/version.h
  )
target_precompile_headers(${PROJECT_NAME} PRIVATE
  ${JSON_SRC_DIR}/include/nlohmann/json.hpp
  ${MAGIC_ENUM_SRC_DIR}/include/magic_enum.hpp
  ${SPDLOG_HEADERS}
  )

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif()
