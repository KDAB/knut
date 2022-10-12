include(FetchContent)

FetchContent_Declare(TreeSitterCpp
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-cpp
    GIT_TAG v0.20.0)
FetchContent_MakeAvailable(TreeSitterCpp)

project(TreeSitterCpp LANGUAGES CXX C)

add_library(${PROJECT_NAME} STATIC
  ${treesittercpp_SOURCE_DIR}/src/parser.c
  ${treesittercpp_SOURCE_DIR}/src/scanner.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE TreeSitter)
