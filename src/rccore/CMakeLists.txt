project(knut-rccore LANGUAGES CXX)

set(PROJECT_SOURCES
    data.h
    data.cpp
    lexer.h
    lexer.cpp
    rcfile.h
    rc_convert.cpp
    rc_parse.cpp
    rc_utility.cpp
    rc_write.cpp
    stream.h
    stream.cpp
    uiwriter.h
    uiwriter.cpp
    rccore.qrc)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(
  ${PROJECT_NAME}
  kdalgorithms
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Qml
  Qt${QT_VERSION_MAJOR}::QmlPrivate
  Qt${QT_VERSION_MAJOR}::Gui
  knut-utils)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
