# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.20)
find_package(Zephyr)
project(attitude_estimation)

option(BUILD_EXPERIMENT "Build experiment programs" OFF)

add_subdirectory(lib)


if ("${BOARD}" STREQUAL "qemu_cortex_m3")
    target_sources(app PRIVATE src/main_qemu.cpp src/settings.cpp)
elseif(BUILD_EXPERIMENT)
    target_sources(app PRIVATE src/main_experiment.cpp)
else ()
    target_sources(app PRIVATE src/main.cpp src/sensors.cpp src/settings.cpp src/pwm.cpp)
    
endif()

target_link_libraries(app PRIVATE attitude_estimation)

# disable exceptions
# to mitigate problems with static variable initialization guards
# call __cxa_guard_acquire and __cxa_guard_release
# see https://github.com/bblanchon/ArduinoJson/issues/356
# not thread safe anymore :(
target_compile_options(app BEFORE PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>)

set_property(TARGET app PROPERTY CXX_STANDARD 17)
