# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(knut-rcui LANGUAGES CXX)

set(PROJECT_SOURCES
    menumodel.h
    menumodel.cpp
    stringmodel.cpp
    stringmodel.h
    toolbarmodel.cpp
    toolbarmodel.h
    acceleratormodel.cpp
    acceleratormodel.h
    assetmodel.cpp
    assetmodel.h
    datamodel.cpp
    datamodel.h
    dialogmodel.cpp
    dialogmodel.h
    includemodel.cpp
    includemodel.h
    rcviewer_global.h
    rcfileview.h
    rcfileview.cpp
    rcfileview.ui)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(
  ${PROJECT_NAME}
  Qt::Core
  Qt::Gui
  Qt::Widgets
  Qt::UiTools
  knut-rccore
  knut-utils)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
