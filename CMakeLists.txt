cmake_minimum_required(VERSION 3.10.0)
project(llvm VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(llvm main.cpp)

include(CTest)
enable_testing()

