project(knut-gui LANGUAGES CXX)

set(PROJECT_SOURCES
    knutmain.h
    knutmain.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    knut-core
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
