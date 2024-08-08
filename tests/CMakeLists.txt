# This file is part of Knut.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

find_program(CLANGD_EXEC clangd)
if(NOT CLANGD_EXEC)
  add_definitions(-DNO_CLANGD=true)
else()
  execute_process(
    COMMAND ${CLANGD_EXEC} --version
    OUTPUT_VARIABLE CLANGD_VERSION_TEXT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" CLANGD_VERSION
               ${CLANGD_VERSION_TEXT})
  if(${CLANGD_VERSION} VERSION_LESS "14.0.0")
    add_definitions(-DOBSOLETE_CLANGD=true)
  endif()
endif()

# * Create a test, pass arbitrary sources and libs
function(add_knut_test_variadic name)
  cmake_parse_arguments(ARG "" "" "SOURCES;LIBS" ${ARGN})

  add_executable(${name} ${ARG_SOURCES})

  target_link_libraries(${name} PRIVATE Qt::Test knut-core ${ARG_LIBS})
  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

  add_test(${name} ${CMAKE_BINARY_DIR}/bin/${name})
  set_property(TEST ${name} PROPERTY ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

# * Create a test, with one source and arbitrary libs
function(add_knut_test name source)
  add_knut_test_variadic(${name} SOURCES ${source} LIBS ${ARGN})
endfunction()

add_knut_test(tst_jsonify tst_jsonify.cpp nlohmann_json::nlohmann_json)

add_knut_test(tst_clientbackend tst_clientbackend.cpp knut-lsp)

add_knut_test(tst_client tst_client.cpp knut-lsp)

add_knut_test(tst_settings tst_settings.cpp)

add_knut_test(tst_stringutils tst_stringutils.cpp)

add_knut_test(tst_textdocument tst_textdocument.cpp)

add_knut_test(tst_rclexer tst_rclexer.cpp knut-rccore)

add_knut_test(tst_rcparser tst_rcparser.cpp knut-rccore)

add_knut_test(tst_rcwriter tst_rcwriter.cpp knut-rccore Qt::UiTools)

add_knut_test(tst_qtuidocument tst_qtuidocument.cpp)

add_knut_test(tst_cppdocument tst_cppdocument.cpp)
add_knut_test(tst_cppdocument_clangd tst_cppdocument_clangd.cpp)
add_knut_test(tst_cppdocument_treesitter tst_cppdocument_treesitter.cpp)

add_knut_test(tst_codedocument tst_codedocument.cpp)

add_knut_test(tst_qmldocument tst_qmldocument.cpp)

add_knut_test(tst_symbol tst_symbol.cpp)

add_knut_test(tst_treesitter tst_treesitter.cpp knut-treesitter)

add_knut_test(tst_qttsdocument tst_qttsdocument.cpp)

# tst_knut is the integration test for the knut executable. It invokes the knut
# executable, instead of instantiating its own KnutCore instance. Therefore, it
# needs to depend on the knut executable, and know the full path to the
# executable.
add_knut_test(tst_knut tst_knut.cpp)
add_dependencies(tst_knut knut)
target_compile_definitions(tst_knut
                           PRIVATE KNUT_BINARY_PATH="${KNUT_BINARY_PATH}")
