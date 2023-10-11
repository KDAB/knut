# TreeSitter
###########################################################
project(TreeSitter LANGUAGES C)

add_library(${PROJECT_NAME} STATIC
tree-sitter/lib/src/lib.c)
target_include_directories(${PROJECT_NAME}
  PUBLIC tree-sitter/lib/include)


# TreeSitterCpp
###########################################################
project(TreeSitterCpp LANGUAGES C)

add_library(${PROJECT_NAME} STATIC
  tree-sitter-cpp/src/parser.c
  tree-sitter-cpp/src/scanner.c)
target_link_libraries(${PROJECT_NAME} PRIVATE TreeSitter)
