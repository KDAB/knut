project(knut VERSION 4.0 LANGUAGES CXX)

set(PROJECT_SOURCES
	main.cpp
	lsp/client.cpp
	lsp/client.h
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets spdlog::spdlog)
