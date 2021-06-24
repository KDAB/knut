project(knut-gui LANGUAGES CXX)

set(PROJECT_SOURCES
    fileselector.h
    fileselector.cpp
    knutmain.h
    knutmain.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    rctoqrcdialog.h
    rctoqrcdialog.cpp
    rctoqrcdialog.ui
    rctouidialog.h
    rctouidialog.cpp
    rctouidialog.ui
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt::Core
    Qt::Widgets
    knut-core
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
