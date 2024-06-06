# Submodule handling
function(check_submodule name path)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${path}/.git")
        if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
            message(FATAL_ERROR
                "The ${name} git submodule is not initialized.\n"
                "Please run the following commands in the source directory (${PROJECT_SOURCE_DIR}):\n"
                "    git submodule update --init --recursive\n"
            )
        else()
            message(FATAL_ERROR
                "The ${name} submodule is missing - please report a broken source package.\n"
            )
        endif()
    endif()
endfunction()

function(check_optional_submodule name path)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${path}/.git")
        message(WARNING
            "The optional ${name} submodule is missing.\n"
        )
    endif()
endfunction()
