cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CXX_STANDARD_REQUIRED 20)

project(file-compresor)


include_directories(src)
add_compile_options(
    "-std=c++23"
    "-Wall"
    "-Wpedantic"
    "-Werror"
)
add_link_options(
    "-static"
)
add_executable(${PROJECT_NAME} src/compressor.cpp src/program.cpp)
add_executable(01-${PROJECT_NAME}-test test/arguments.cpp src/compressor.cpp)
add_executable(02-${PROJECT_NAME}-test test/file_content_struct_test.cpp src/compressor.cpp)