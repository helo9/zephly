cmake_minimum_required(VERSION 3.20)

project(AHRS)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    find_package(Catch2 3 REQUIRED)

    add_executable(tests tests.cpp)
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

    add_library(ahrs ahrs.cpp)

    add_executable(example example.c)
    target_link_libraries(example PRIVATE ahrs)

else ()

    add_library(ahrs INTERFACE)
    target_include_directories(ahrs INTERFACE .)

endif()
