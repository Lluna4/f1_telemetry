cmake_minimum_required(VERSION 3.5.0)
project(f1_read VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(frozen)

add_compile_options(/std:c++latest)

add_executable(f1_read CMakeProject2.cpp)

target_link_libraries(f1_read PRIVATE frozen::frozen)