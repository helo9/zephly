# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.20)

find_package(Zephyr)
project(mpu6000_sample)

target_sources(app PRIVATE src/main.c)
