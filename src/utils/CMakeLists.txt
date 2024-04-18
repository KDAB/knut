project(knut-utils LANGUAGES CXX)
find_package(Qt6 REQUIRED COMPONENTS Core Gui)
set(PROJECT_SOURCES json.h qt_fmt_helpers.h strings.h strings.cpp)
add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
if(KNUT_USE_STD_FORMAT)
  target_sources(${PROJECT_NAME} PUBLIC qt_std_format.h)
else()
  target_sources(${PROJECT_NAME} PUBLIC qt_fmt_format.h)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json
                                             Qt6::Core Qt6::Gui)

target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
