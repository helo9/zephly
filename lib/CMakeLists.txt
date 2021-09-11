cmake_minimum_required(VERSION 3.20)

project(AHRS)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    find_package(xtl REQUIRED)
    find_package(xtensor REQUIRED)
    find_package(Catch2 3 REQUIRED)

    add_executable(tests tests.cpp)
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(tests PRIVATE xtensor)

    add_library(ahrs ahrs.cpp)

    add_executable(example example.c)
    target_link_libraries(example PRIVATE ahrs)
    
    find_package(Python COMPONENTS Interpreter Development)
    find_package(pybind11 CONFIG)
    
    pybind11_add_module(pyahrs python_binding.cpp)
    target_link_libraries(pyahrs PRIVATE ahrs)
else ()

    add_library(ahrs INTERFACE)
    target_include_directories(ahrs INTERFACE .)

endif()
