# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(attitude_estimation)


target_sources(app PRIVATE 
    src/main.cpp
    src/zephly_sensors.c
    src/zephly_sensors_config.c
)

# disable exceptions
# to mitigate problems with static variable initialization guards
# call __cxa_guard_acquire and __cxa_guard_release
# see https://github.com/bblanchon/ArduinoJson/issues/356
# not thread safe anymore :(
target_compile_options(app BEFORE PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>)

set_property(TARGET app PROPERTY CXX_STANDARD 17)
