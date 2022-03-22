# Copyright (c) 2022 Jonathan Hahn
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources(
    simulator_connection.c
    simulator_connection_mavlink.c
    simulator_connection_network.c
)
