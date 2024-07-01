# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

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
    runscriptwidget.h
    runscriptwidget.cpp
    runscriptwidget.ui
    scriptpanel.h
    scriptpanel.cpp
    scriptlistpanel.cpp
    scriptlistpanel.h
    shortcutmanager.h
    shortcutmanager.cpp
    shortcutsettings.h
    shortcutsettings.cpp
    shortcutsettings.ui
    slintview.h
    slintview.cpp
    textview.h
    textview.cpp
    toolbar.h
    toolbar.cpp
    qtuiview.h
    qtuiview.cpp
    qmlview.h
    qmlview.cpp
    qttsview.h
    qttsview.cpp
    gui.qrc)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC Qt::Core
         Qt::Widgets
         Qt::UiTools
         knut-core
         knut-rcui
         knut-utils
         KF5SyntaxHighlighting)

target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
