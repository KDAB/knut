project(knut-rccore LANGUAGES CXX)

set(PROJECT_SOURCES
    data.h
    data.cpp
    lexer.h
    lexer.cpp
    rcfile.h
    rc_convert.cpp
    rc_parse.cpp
    rc_write.cpp
    rc_utils.h
    rc_utils.cpp
    stream.h
    stream.cpp
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME}
    spdlog::spdlog
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
