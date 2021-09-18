# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.20)
find_package(Zephyr)
project(attitude_estimation)

add_subdirectory(lib)

if (NOT "${BOARD}" STREQUAL "qemu_cortex_m3")
    # Add your source file to the "app" target. This must come after
    # find_package(Zephyr) which defines the target.
    target_sources(app PRIVATE src/main.cpp src/sensors.cpp src/settings.cpp)
else ()
    target_sources(app PRIVATE src/main_qemu.cpp src/settings.cpp)
endif()

target_link_libraries(app PRIVATE ahrs)

# disable exceptions
# to mitigate problems with static variable initialization guards
# call __cxa_guard_acquire and __cxa_guard_release
# see https://github.com/bblanchon/ArduinoJson/issues/356
# not thread safe anymore :(
target_compile_options(app BEFORE PUBLIC -fno-threadsafe-statics)
set_property(TARGET app PROPERTY CXX_STANDARD 17)
