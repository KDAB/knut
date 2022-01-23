project(rcviewer VERSION 1 LANGUAGES CXX)

set(PROJECT_SOURCES
    rcviewer.cpp
    mainwindow.h
    mainwindow.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    knut-rcui
)
