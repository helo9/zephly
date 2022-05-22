# Copyright (c) 2022 Jonathan Hahn
# SPDX-License-Identifier: Apache-2.0

# add_subdirectory(attitude_estimation)
add_subdirectory_ifdef(CONFIG_ZEPHLY_MIXER_LIB mixer)
add_subdirectory_ifdef(CONFIG_ZEPHLY_CONTROL_LIB control)
add_subdirectory_ifdef(CONFIG_ZEPHLY_SIMULATOR_CONNECTION simulator_connection)
