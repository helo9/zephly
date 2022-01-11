/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/uart.h>
#include <stdint.h>

#ifndef FFC_RC_H
#define FFC_RC_H

struct rc_input {
    uint16_t roll;
    uint16_t pitch;
    uint16_t throttle;
    uint16_t yaw;
};

struct rc_api {
	void (*update)(const struct device *dev, struct rc_input *rc_in);
};

/**
 * @brief update rc_in struct with values from receiver device
 * 
 * @param dev receiver device
 * @param rc_in pointer to rc input to be updated
 */
static inline void rc_update(const struct device *dev, struct rc_input *rc_in) {
    const struct rc_api *api = dev->api;
    api->update(dev, rc_in);
}

#endif // FFC_RC_H
