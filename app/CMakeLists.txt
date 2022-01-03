# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.20)

find_package(Zephyr)
message(INFO "jojojo ${BOARD_ROOT}")

project(attitude_estimation)

if(BUILD_EXPERIMENT)
    target_sources(app PRIVATE src/main_experiment_spi.c)
else ()
    target_sources(app PRIVATE 
        src/main.cpp
        src/sensors.cpp
        src/pwm.cpp 
        src/rc/rc.cpp
        src/rc/rc_thread.cpp
        src/rc/spm_srxl.c
    )
endif()

#target_link_libraries(app PRIVATE attitude_estimation msgs)

# disable exceptions
# to mitigate problems with static variable initialization guards
# call __cxa_guard_acquire and __cxa_guard_release
# see https://github.com/bblanchon/ArduinoJson/issues/356
# not thread safe anymore :(
target_compile_options(app BEFORE PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>)

set_property(TARGET app PROPERTY CXX_STANDARD 17)
