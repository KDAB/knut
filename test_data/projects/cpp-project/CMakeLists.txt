# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

cmake_minimum_required(VERSION 3.10)

project(KnutExampleCppProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add the executable
add_executable(KnutExampleCppProject main.cpp myobject.cpp)

