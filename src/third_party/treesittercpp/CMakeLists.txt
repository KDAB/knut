include(FetchContent)

FetchContent_Declare(TreeSitterCpp
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-cpp
    # Currently, bumping TreeSitterCpp to 0.20.3 causes the Parser
    # to get hung up. 0.20.2 is the latest version that works as of 2023-Oct-11
    # This may be related to: https://github.com/tree-sitter/tree-sitter/issues/2684
    GIT_TAG v0.20.2)
FetchContent_MakeAvailable(TreeSitterCpp)

project(TreeSitterCpp LANGUAGES C)

add_library(${PROJECT_NAME} STATIC
  ${treesittercpp_SOURCE_DIR}/src/parser.c
  ${treesittercpp_SOURCE_DIR}/src/scanner.c)
target_link_libraries(${PROJECT_NAME} PRIVATE TreeSitter)
