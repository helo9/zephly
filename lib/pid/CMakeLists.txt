cmake_minimum_required(VERSION 3.20)

project(pid)

option(BUILD_TESTS "Build test programs" OFF)

add_library(pid STATIC pid.cpp)
target_include_directories(pid INTERFACE .)

if(BUILD_TESTS)
    find_package(Catch2 3 REQUIRED)

    add_executable(test test_pid.cpp)
    target_link_libraries(test PRIVATE Catch2::Catch2WithMain pid)
endif()
