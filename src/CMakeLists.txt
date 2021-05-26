project(knut LANGUAGES CXX)

set(PROJECT_SOURCES
    main.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    knut-core
    knut-ui
)

add_subdirectory(core)
add_subdirectory(lsp)
add_subdirectory(ui)
