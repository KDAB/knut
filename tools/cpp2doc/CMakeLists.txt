project(cpp2doc VERSION 1 LANGUAGES CXX)

add_definitions(-DKNUT_SOURCE_PATH="${CMAKE_SOURCE_DIR}/src")
add_definitions(-DKNUT_DOC_PATH="${CMAKE_SOURCE_DIR}/docs")

set(PROJECT_SOURCES
    data.h
    cpp2doc.cpp
    sourceparser.cpp
    sourceparser.h
    docwriter.h
    docwriter.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)
