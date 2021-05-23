find_program(CLANGD_EXEC clangd)
if (NOT CLANGD_EXEC)
    add_definitions(-DNO_CLANGD=true)
endif()

function(add_knut_test NAME SOURCES LIBS)
    add_executable(${NAME} ${SOURCES} common/test_utils.h)

    target_link_libraries(${NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Test
        ${LIBS}
    )
    target_include_directories(${NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src
    )

    add_test(${NAME} ${CMAKE_BINARY_DIR}/bin/${NAME})
endfunction()

add_knut_test(tst_jsonify tst_jsonify.cpp nlohmann_json::nlohmann_json)

add_knut_test(tst_clientbackend tst_clientbackend.cpp knut-lsp)
add_knut_test(tst_client tst_client.cpp knut-lsp)

add_knut_test(tst_settings tst_settings.cpp knut-core)
