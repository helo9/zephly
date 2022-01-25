/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include "kernel.h"
#include <device.h>
#include <devicetree.h>
#include <stdio.h>
#include <custom_drivers/rc.h>
#include <mixer/mixer.hpp>

#include "pwm.hpp"

#define RC_IN DT_NODELABEL(rc)

const struct device *rc = DEVICE_DT_GET(RC_IN);

/* initial pwm values */
float outputs[] = {0.0f, 0.0f, 0.0f, 0.0f};
struct Command rc_val;

int main() {
	printk("Initialization started.\n");

	if (!device_is_ready(rc)) {
		printk("RC IN device not ready.");
		return -EIO;
	}

	auto &pwm = PWM::init();

	/* write initial pwm values */
	pwm.write(outputs);

	printk("Initialization complete.\nRunning!\n");

	while (true) {
		rc_update(rc, &rc_val);

		px_airmode_mix(rc_val, outputs);
		
		pwm.write(outputs);

		k_sleep(K_MSEC(10));
	}

	return 0;
}
