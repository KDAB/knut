# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(knut LANGUAGES CXX)

set(PROJECT_SOURCES main.cpp)

if(WIN32)
  set(PROJECT_ICON knut.rc)
endif()

add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32 ${PROJECT_SOURCES}
                                                   ${PROJECT_ICON})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets knut-gui)

install(
  TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}"
)

add_subdirectory(core)
add_subdirectory(lsp)
add_subdirectory(treesitter)
add_subdirectory(rccore)
add_subdirectory(rcui)
add_subdirectory(gui)
add_subdirectory(utils)
