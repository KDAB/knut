project(knut VERSION 4.0 LANGUAGES CXX)

set(PROJECT_SOURCES
    main.cpp
    core/scriptitem.cpp
    core/scriptitem.h
    core/settings.cpp
    core/settings.h
    core/dir.cpp
    core/dir.h
    core/file.cpp
    core/file.h
    core/fileinfo.cpp
    core/fileinfo.h
    core/qdirvaluetype.cpp
    core/qdirvaluetype.h
    core/qfileinfovaluetype.cpp
    core/qfileinfovaluetype.h
    lsp/client.cpp
    lsp/client.h
    lsp/clientbackend.cpp
    lsp/clientbackend.h
    lsp/notifications.h
    lsp/notificationmessage_json.h
    lsp/requestmessage.h
    lsp/requestmessage_json.h
    lsp/requests.h
    lsp/types.h
    lsp/types_json.h
    lsp/types_json.cpp
    lsp/notificationmessage.h
    lsp/lsp_utils.h
    script/scriptmanager.cpp
    script/scriptmanager.h
    script/scriptrunner.cpp
    script/scriptrunner.h
    script/testutil.cpp
    script/testutil.h
    utils/json_utils.h
    utils/json_utils.cpp
    utils/test_utils.h
)

qt5_add_resources(PROJECT_QRC
    resources/resources.qrc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_QRC})
add_test(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::QmlPrivate
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    doctest::doctest
    cxxopts
)

FetchContent_GetProperties(json SOURCE_DIR JSON_SRC_DIR)
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
    ${SPDLOG_HEADERS}
)
