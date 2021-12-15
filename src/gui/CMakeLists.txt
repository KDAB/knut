project(knut-gui LANGUAGES CXX)

set(PROJECT_SOURCES
    fileselector.h
    fileselector.cpp
    knutmain.h
    knutmain.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    palette.h
    palette.cpp
    palette.ui
    rctoqrcdialog.h
    rctoqrcdialog.cpp
    rctoqrcdialog.ui
    rctouidialog.h
    rctouidialog.cpp
    rctouidialog.ui
    runscriptdialog.h
    runscriptdialog.cpp
    runscriptdialog.ui
    settingsdialog.h
    settingsdialog.cpp
    settingsdialog.ui
    uiview.h
    uiview.cpp
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt::Core
    Qt::Widgets
    Qt::UiTools
    KF5SyntaxHighlighting
    knut-core
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
