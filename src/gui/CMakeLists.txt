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
    rcui/menumodel.h
    rcui/menumodel.cpp
    rcui/stringmodel.cpp
    rcui/stringmodel.h
    rcui/toolbarmodel.cpp
    rcui/toolbarmodel.h
    rcui/acceleratormodel.cpp
    rcui/acceleratormodel.h
    rcui/assetmodel.cpp
    rcui/ribbonmodel.cpp
    rcui/assetmodel.h
    rcui/ribbonmodel.h
    rcui/datamodel.cpp
    rcui/datamodel.h
    rcui/dialogmodel.cpp
    rcui/dialogmodel.h
    rcui/includemodel.cpp
    rcui/includemodel.h
    rcui/rcviewer_global.h
    rcui/rcfileview.h
    rcui/rcfileview.cpp
    rcui/rcfileview.ui
    apiexecutorwidget.h
    apiexecutorwidget.cpp
    apiexecutorwidget.ui
    codeview.cpp
    codeview.h
    documentpalette.h
    documentpalette.cpp
    fileselector.h
    fileselector.cpp
    findadapter.h
    findadapter.cpp
    findinterface.h
    findwidget.h
    findinfilespanel.cpp
    findinfilespanel.h
    findwidget.cpp
    findwidget.ui
    gui_constants.h
    guisettings.h
    guisettings.cpp
    highlightdelegate.h
    highlightdelegate.cpp
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
    knuttabwidget.h
    knuttabwidget.cpp
    logpanel.h
    logpanel.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    optionsdialog.h
    optionsdialog.cpp
    optionsdialog.ui
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
         knut-rccore
         knut-utils
         KF6SyntaxHighlighting)

target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
