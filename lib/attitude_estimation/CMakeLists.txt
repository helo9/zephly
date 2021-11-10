cmake_minimum_required(VERSION 3.20)

project(controllib)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    find_package(xtl REQUIRED)
    find_package(xtensor REQUIRED)
    find_package(Catch2 3 REQUIRED)

    add_executable(tests test_attitude_estimation.cpp)
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(tests PRIVATE xtensor)

    find_package(Python COMPONENTS Interpreter Development)
    find_package(pybind11 CONFIG)
    
    pybind11_add_module(pyahrs examples/attitude_estimation_py.cpp)
    target_include_directories(pyahrs PRIVATE .)
else ()

    add_library(attitude_estimation INTERFACE)
    target_include_directories(attitude_estimation INTERFACE .)

endif()
