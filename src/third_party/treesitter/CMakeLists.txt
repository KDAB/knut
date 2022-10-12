include(FetchContent)

FetchContent_Declare(TreeSitter
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
    GIT_TAG v0.20.7
    )
FetchContent_MakeAvailable(TreeSitter)

project(TreeSitter LANGUAGES CXX C)

add_library(${PROJECT_NAME} STATIC
  ${treesitter_SOURCE_DIR}/lib/src/lib.c)
target_include_directories(${PROJECT_NAME}
  PUBLIC ${treesitter_SOURCE_DIR}/lib/include)
