project(spec2cpp VERSION 1 LANGUAGES CXX)

add_definitions(-DLSP_SOURCE_PATH="${CMAKE_SOURCE_DIR}/src/lsp")

set(PROJECT_SOURCES
    data.h
    spec2cpp.cpp
    specparser.h
    specparser.cpp
    specwriter.h
    specwriter.cpp
)

qt_add_resources(PROJECT_SOURCES
    spec2cpp.qrc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)
