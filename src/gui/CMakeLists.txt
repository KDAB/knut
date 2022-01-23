project(knut-gui LANGUAGES CXX)

set(PROJECT_SOURCES
    fileselector.h
    fileselector.cpp
    imageview.h
    imageview.cpp
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
    gui.qrc
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::UiTools
    KF5SyntaxHighlighting
    knut-core
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
