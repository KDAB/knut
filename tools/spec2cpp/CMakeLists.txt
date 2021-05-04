project(spec2cpp VERSION 4.0 LANGUAGES CXX)

set(PROJECT_SOURCES
    spec2cpp.cpp
    specification.h
)

qt5_add_resources(PROJECT_QRC
    spec2cpp.qrc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_QRC})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif()
