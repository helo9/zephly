cmake_minimum_required(VERSION 3.20)

project(mixerlib)

option(BUILD_TESTS "Build test programs" OFF)

add_library(mixer STATIC mixer.cpp)
target_include_directories(mixer INTERFACE .)

if(BUILD_TESTS)
    find_package(Catch2 3 REQUIRED)

    add_executable(test test_mixer.cpp)
    target_link_libraries(test PRIVATE Catch2::Catch2WithMain mixer)
endif()
