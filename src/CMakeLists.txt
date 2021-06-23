project(knut LANGUAGES CXX)

set(PROJECT_SOURCES
    main.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    knut-gui
)

add_subdirectory(core)
add_subdirectory(lsp)
add_subdirectory(rccore)
add_subdirectory(rcui)
add_subdirectory(gui)
