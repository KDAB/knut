find_program(CLANGD_EXEC clangd)
if (NOT CLANGD_EXEC)
    add_definitions(-DNO_CLANGD=true)
else()
    execute_process(COMMAND ${CLANGD_EXEC} --version
        OUTPUT_VARIABLE CLANGD_VERSION_TEXT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" CLANGD_VERSION ${CLANGD_VERSION_TEXT})
    if (${CLANGD_VERSION} VERSION_LESS  "14.0.0")
        add_definitions(-DOBSOLETE_CLANGD=true)
    endif()
endif()

function(add_knut_test_variadic NAME)
    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;LIBS"
        ${ARGN})

    add_executable(${NAME} ${ARG_SOURCES})

    target_link_libraries(${NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        ${ARG_LIBS}
    )
    target_include_directories(${NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
    )

    add_test(${NAME} ${CMAKE_BINARY_DIR}/bin/${NAME})
endfunction()

function(add_knut_test NAME SOURCE)
    add_knut_test_variadic(${NAME} SOURCES ${SOURCE} LIBS ${ARGN})
endfunction()

add_knut_test(tst_jsonify tst_jsonify.cpp nlohmann_json::nlohmann_json)

add_knut_test(tst_clientbackend tst_clientbackend.cpp knut-lsp)

add_knut_test(tst_client tst_client.cpp knut-lsp)

add_knut_test(tst_settings tst_settings.cpp knut-core)

add_knut_test_variadic(tst_knut SOURCES tst_knut.cpp common/testutil.cpp common/testutil.h tests.qrc LIBS knut-core)
file(GLOB QML_FILES ../test_data/*.qml)
add_custom_target(tst_knut_data SOURCES ${QML_FILES})

add_knut_test(tst_stringutils tst_stringutils.cpp knut-core)

add_knut_test(tst_textdocument tst_textdocument.cpp knut-core)

add_knut_test(tst_rclexer tst_rclexer.cpp knut-rccore)

add_knut_test(tst_rcparser tst_rcparser.cpp knut-rccore)

add_knut_test(tst_rcwriter tst_rcwriter.cpp knut-rccore Qt::UiTools)

add_knut_test(tst_uidocument tst_uidocument.cpp knut-core)

add_knut_test(tst_cppdocument tst_cppdocument.cpp knut-core)
add_knut_test(tst_cppdocument_clangd tst_cppdocument_clangd.cpp knut-core)
add_knut_test(tst_cppdocument_treesitter tst_cppdocument_treesitter.cpp knut-core)

add_knut_test(tst_lspdocument tst_lspdocument.cpp knut-core)

add_knut_test(tst_symbol tst_symbol.cpp knut-core)

add_knut_test(tst_treesitter tst_treesitter.cpp knut-core knut-treesitter)

add_knut_test(tst_photon tst_photon.cpp knut-core)
