
# Copyright (c) 2022 Jonathan Hahn
# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_MPU6000_DRIVER mpu6000)
add_subdirectory_ifdef(CONFIG_ICM20602_DRIVER icm20602)
add_subdirectory(rc)
add_subdirectory_ifdef(CONFIG_CONTROL_OUTPUT_PWM control_output/pwm)
