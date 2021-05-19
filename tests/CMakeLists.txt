if(WIN32)
    add_definitions(-DKNUT_EXE="${CMAKE_BINARY_DIR}/bin/knut.exe")
else()
    add_definitions(-DKNUT_EXE="${CMAKE_BINARY_DIR}/bin/knut")
endif()

function(add_knut_test NAME SOURCES)
    add_executable(${NAME} ${SOURCES})

    target_link_libraries(${NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        doctest::doctest
    )

    add_test(${NAME} ${CMAKE_BINARY_DIR}/bin/${NAME})
endfunction()

add_knut_test(tst_knut tst_knut.cpp)
