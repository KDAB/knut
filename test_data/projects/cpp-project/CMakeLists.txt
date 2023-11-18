cmake_minimum_required(VERSION 3.10)

project(KnutExampleCppProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add the executable
add_executable(KnutExampleCppProject main.cpp myobject.cpp)

