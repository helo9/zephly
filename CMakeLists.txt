# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.20)

option(BUILD_EXPERIMENT "Build experiment programs" OFF)
option(BUILD_TESTING "Build tests" OFF)

if(BUILD_TESTING)
    add_subdirectory(lib)
else ()

    set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
    find_package(Zephyr)
    project(attitude_estimation)


    add_subdirectory(lib)

    if(BUILD_EXPERIMENT)
        target_sources(app PRIVATE src/main_experiment2.cpp src/rc/rc.cpp)
    else ()
        target_sources(app PRIVATE src/main.cpp src/sensors.cpp src/pwm.cpp)
        
    endif()

    target_link_libraries(app PRIVATE attitude_estimation msgs)

    # disable exceptions
    # to mitigate problems with static variable initialization guards
    # call __cxa_guard_acquire and __cxa_guard_release
    # see https://github.com/bblanchon/ArduinoJson/issues/356
    # not thread safe anymore :(
    target_compile_options(app BEFORE PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>)

    set_property(TARGET app PROPERTY CXX_STANDARD 17)
endif()
