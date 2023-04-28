project(knut-gui LANGUAGES CXX)

set(PROJECT_SOURCES
    apiexecutorwidget.h
    apiexecutorwidget.cpp
    apiexecutorwidget.ui
    documentpalette.h
    documentpalette.cpp
    fileselector.h
    fileselector.cpp
    findwidget.h
    findwidget.cpp
    findwidget.ui
    gui_constants.h
    guisettings.h
    guisettings.cpp
    historypanel.h
    historypanel.cpp
    imageview.h
    imageview.cpp
    interfacesettings.h
    interfacesettings.cpp
    interfacesettings.ui
    knutmain.h
    knutmain.cpp
    knutstyle.h
    knutstyle.cpp
    logpanel.h
    logpanel.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    optionsdialog.h
    optionsdialog.cpp
    optionsdialog.ui
    transformpreviewdialog.h
    transformpreviewdialog.cpp
    transformpreviewdialog.ui
    treesitterinspector.h
    treesitterinspector.cpp
    treesitterinspector.ui
    treesittertreemodel.h
    treesittertreemodel.cpp
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
    scriptpanel.h
    scriptpanel.cpp
    shortcutmanager.h
    shortcutmanager.cpp
    shortcutsettings.h
    shortcutsettings.cpp
    shortcutsettings.ui
    textview.h
    textview.cpp
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
