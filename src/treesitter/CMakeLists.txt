project(knut-treesitter LANGUAGES CXX)

set(PROJECT_SOURCES node.cpp parser.cpp predicates.cpp query.cpp
                    transformation.cpp tree.cpp)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} TreeSitter TreeSitterCpp kdalgorithms
                      spdlog::spdlog Qt${QT_VERSION_MAJOR}::Core)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
