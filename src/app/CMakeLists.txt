set(PROJECT_SOURCES
        main.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(knut
        ${PROJECT_SOURCES}
    )
else()
    add_executable(knut
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(knut PRIVATE Qt${QT_VERSION_MAJOR}::Widgets spdlog::spdlog)
