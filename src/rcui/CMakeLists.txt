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
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::UiTools
    knut-rccore
)
target_include_directories(${PROJECT_NAME}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..
)
