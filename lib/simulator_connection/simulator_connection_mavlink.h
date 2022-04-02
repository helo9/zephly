/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHLY_SIMULATOR_MAVLINK
#define ZEPHLY_SIMULATOR_MAVLINK

#include <stdint.h>

#include <common/mavlink.h>

uint16_t simulator_write_mavlink_hearbeat_buffer(uint8_t *buffer);

uint16_t simulator_write_mavlink_output_to_buffer(const float outputs[4], uint8_t *buffer);

int simulator_parse_message(uint8_t *buffer, size_t buffer_len, float gyro[3]);

#endif /* ZEPHLY_SIMULATOR_MAVLINK */
