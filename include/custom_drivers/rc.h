/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/uart.h>
#include <stdint.h>
#include <msgs/msgs.h>

#ifndef FFC_RC_H
#define FFC_RC_H

struct rc_api {
	void (*update)(const struct device *dev, struct Command *rc_in);
};

/**
 * @brief update rc_in struct with values from receiver device
 * 
 * @param dev receiver device
 * @param rc_in pointer to rc input to be updated
 */
static inline void rc_update(const struct device *dev, struct Command *rc_in) {
    const struct rc_api *api = (struct rc_api*)dev->api;
    api->update(dev, rc_in);
}

#endif // FFC_RC_H
