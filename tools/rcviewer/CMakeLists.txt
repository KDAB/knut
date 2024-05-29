# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(
  rcviewer
  VERSION 1
  LANGUAGES CXX)

set(PROJECT_SOURCES rcviewer.cpp mainwindow.h mainwindow.cpp)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(
  ${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core
                          Qt${QT_VERSION_MAJOR}::Widgets knut-rcui)
