project(spec2cpp VERSION 4.0 LANGUAGES CXX)

set(PROJECT_SOURCES
    spec2cpp.cpp
    specparser.cpp
    specparser.h
    specwriter.cpp
    specwriter.h
    data.h
)

qt5_add_resources(PROJECT_QRC
    spec2cpp.qrc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_QRC})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)
