#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(INTERNAL_DEPENDENCIES_PREFIX "${CMAKE_BINARY_DIR}/_deps/prefix")
list(PREPEND CMAKE_PREFIX_PATH "${INTERNAL_DEPENDENCIES_PREFIX}")

file(MAKE_DIRECTORY "${INTERNAL_DEPENDENCIES_PREFIX}")

function(install_dependency NAME SOURCE_DIR CMAKE_ARGS)
    set(BUILD_DIR "${CMAKE_BINARY_DIR}/_deps/${NAME}")
    file(MAKE_DIRECTORY "${BUILD_DIR}")

    message("Configuring dependency ${NAME}")

    list(JOIN CMAKE_PREFIX_PATH "\;" CMAKE_PREFIX_PATH_ESCAPED)

    list(PREPEND CMAKE_ARGS
        "-G${CMAKE_GENERATOR}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_ESCAPED}"
        "-DCMAKE_INSTALL_PREFIX=${INTERNAL_DEPENDENCIES_PREFIX}"
        "-DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}"
        "-DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}"
        "-DKDE_INSTALL_PLUGINDIR=${KDE_INSTALL_PLUGINDIR}"
        "-DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}"
        "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}"
    )

    if (CMAKE_TOOLCHAIN_FILE)
        list(APPEND CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    else()
        list(APPEND CMAKE_ARGS "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
        list(APPEND CMAKE_ARGS "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
    endif()

    execute_process(
        COMMAND "${CMAKE_COMMAND}" ${CMAKE_ARGS} "${SOURCE_DIR}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_QUIET
        COMMAND_ERROR_IS_FATAL ANY
    )

    message("Building dependency ${NAME} in ${BUILD_DIR}")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_QUIET
        COMMAND_ERROR_IS_FATAL ANY
    )

    message("Installing dependency ${NAME} in ${CMAKE_INSTALL_PREFIX}")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" --install .
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_QUIET
        COMMAND_ERROR_IS_FATAL ANY
    )

    install(CODE "
        message(\"Installing dependency ${NAME} to final prefix\")
        execute_process(
            COMMAND \"${CMAKE_COMMAND}\" --install . --prefix \"${CMAKE_INSTALL_PREFIX}\"
            WORKING_DIRECTORY \"${BUILD_DIR}\"
            COMMAND_ERROR_IS_FATAL ANY
        )
    ")
endfunction()
