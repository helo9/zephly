# Copyright (c) 2022 Jonathan Hahn
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_MPU6000_DRIVER mpu6000.c)
