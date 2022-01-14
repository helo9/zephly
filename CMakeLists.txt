# Copyright (c) 2022 Jonathan Hahn
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(drivers)
add_subdirectory(lib)

zephyr_include_directories(include)
zephyr_include_directories(lib)
