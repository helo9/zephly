# Copyright (c) 2022 Jonathan Hahn
# SPDX-License-Identifier: Apache-2.0

find_package(SDL2 REQUIRED PATHS /usr/lib/x86_64-linux-gnu/cmake/SDL2)

zephyr_include_directories(.)

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_ZEPHLY_JOYSTICK_RC_DRIVER 
    joystick_rc.c
)
zephyr_library_link_libraries(${SDL2_LIBRARIES})
zephyr_library_include_directories(${SDL2_INCLUDE_DIRS})
