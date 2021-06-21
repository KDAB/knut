project(knut-rcui LANGUAGES CXX)

set(PROJECT_SOURCES
    menumodel.h
    menumodel.cpp
    stringmodel.cpp
    stringmodel.h
    toolbarmodel.cpp
    toolbarmodel.h
    acceleratormodel.cpp
    acceleratormodel.h
    assetmodel.cpp
    assetmodel.h
    datamodel.cpp
    datamodel.h
    dialogmodel.cpp
    dialogmodel.h
    includemodel.cpp
    includemodel.h
    rcviewer_global.h
    rcfileview.h
    rcfileview.cpp
    rcfileview.ui
    rcsyntaxhighlighter.h
    rcsyntaxhighlighter.cpp
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME}
    spdlog::spdlog
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::UiTools
    knut-rcfile
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
