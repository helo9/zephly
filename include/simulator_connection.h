/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHLY_SIMULATOR_CONNECTION_H
#define ZEPHLY_SIMULATOR_CONNECTION_H

#include <stdbool.h>

bool simulator_is_ready();

void simulator_send_outputs(const float outputs[4]);

int simulator_update_sensor_values(float gyro[3]);

#endif
