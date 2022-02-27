project(knut-gui LANGUAGES CXX)

set(PROJECT_SOURCES
    fileselector.h
    fileselector.cpp
    guisettings.h
    guisettings.cpp
    imageview.h
    imageview.cpp
    knutmain.h
    knutmain.cpp
    logpanel.h
    logpanel.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    optionsdialog.h
    optionsdialog.cpp
    optionsdialog.ui
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
    settingswidget.h
    settingswidget.cpp
    settingswidget.ui
    uiview.h
    uiview.cpp
    gui.qrc
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::UiTools
    knut-core
    knut-rcui
)
if(USE_SYNTAX_HIGHLIGHTING)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        KF5SyntaxHighlighting
    )
endif()

target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
