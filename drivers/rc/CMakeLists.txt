
# Copyright (c) 2022 Jonathan Hahn
# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_ZEPHLY_RC_COMMON rc_common)
add_subdirectory_ifdef(CONFIG_ZEPHLY_SRXL2_RC_DRIVER srxl2)
add_subdirectory_ifdef(CONFIG_ZEPHLY_JOYSTICK_RC_DRIVER joystick)
