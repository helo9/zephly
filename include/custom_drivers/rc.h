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

/**
 * @brief Call during initialization of system
 *
 * @details Initializes parser state machine and
 * variables. Also checks whether underlying uart
 * is available and enable interrupts.
 * 
 * @return int 0 or -error
 */
//int rc_init(struct RCInput *rc);

int rc_run_once(const struct device *dev, uint32_t dt);

void rc_update(const struct device *dev, struct rc_input *rc_in);

#endif // FFC_RC_H
