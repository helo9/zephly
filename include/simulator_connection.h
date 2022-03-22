/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHLY_SIMULATOR_CONNECTION_H
#define ZEPHLY_SIMULATOR_CONNECTION_H

#include <stdbool.h>

bool simulator_is_ready();

void simulator_send_outputs(float outputs[4]);

#endif
