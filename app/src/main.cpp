/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include "kernel.h"
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <custom_drivers/rc.h>
#include <mixer/mixer.hpp>
#include <ratecontrol.h>
#include <stdio.h>

#include "pwm.hpp"
#include "zephly_sensors.h"

#define RC_IN DT_NODELABEL(rc)
#define IMU_NODE DT_NODELABEL(imu)

const struct device *rc = DEVICE_DT_GET(RC_IN);

/* initial pwm values */
static float outputs[] = {0.0f, 0.0f, 0.0f, 0.0f};

/* storage for rc inputs */
static struct Command rc_val = {};

static float measurements[3];

static struct RateControl ratecontrols[3];

static void configure_ratecontrol(struct RateControl *ratecontrols);
inline struct Command calculate_setpoint(const struct Command &rc_cmd);
inline struct Command run_ratecontrol(struct RateControl *ratecontrol, const struct Command &setpoints, const float *measruements);

int main() {
	int ret = 0;
	printk("Starting initialization.\n");

	ratecontrol_init(&ratecontrols[0], "pitch");
	ratecontrol_init(&ratecontrols[1], "roll");
	ratecontrol_init(&ratecontrols[2], "yaw");

	configure_ratecontrol(ratecontrols);

	if (!device_is_ready(rc)) {
		printk("RC IN device not ready.");
		return -EIO;
	}

	ret = zephly_sensors_init();
	if (ret != 0) {
		printk("Sensor initialization failed");
		return ret;
	}

	auto &pwm = PWM::init();

	/* write initial pwm values */
	pwm.write(outputs);

	printk("Initialization complete.\nRunning!\n");

	while (true) {
		/* update rc */
		rc_update(rc, &rc_val);

		/* get new sensor measurements */
		zephly_sensors_get_gyro(measurements);

		/* get setpoint from rc_val */
		const auto setpoint = calculate_setpoint(rc_val);
		const auto commands = run_ratecontrol(ratecontrols, setpoint, measurements);
		
		/* run mixer */
		px_airmode_mix(commands, outputs);
		
		/* write outputs */
		pwm.write(outputs);

		k_sleep(K_MSEC(10));
	}

	return 0;
}

static void configure_ratecontrol(struct RateControl *ratecontrols) {
	for (int i=0; i<3; i++) {
		ratecontrols[i].k_p = 1.0f;
		ratecontrols[i].k_i = 0.0f;
		ratecontrols[i].k_d = 0.0f;
	}
}

inline struct Command calculate_setpoint(const struct Command &rc_cmd) {
	static constexpr float MAX_RATE = 1.0f;

	struct Command setpoint;
	setpoint.pitch  = rc_cmd.pitch * MAX_RATE;
	setpoint.roll 	= rc_cmd.roll * MAX_RATE;
	setpoint.yaw	= rc_cmd.yaw * MAX_RATE;
	setpoint.thrust	= rc_cmd.thrust;

	return setpoint;
}

inline struct Command run_ratecontrol(struct RateControl *ratecontrols, const struct Command &setpoints, const float *measruements) {
	struct Command cmd;
	cmd.pitch 	= ratecontrol_update(&ratecontrols[0], setpoints.pitch, measurements[0]);
	cmd.roll	= ratecontrol_update(&ratecontrols[1], setpoints.roll, measurements[1]);
	cmd.yaw 	= ratecontrol_update(&ratecontrols[2], setpoints.yaw, measurements[2]);
	cmd.thrust	= setpoints.thrust;

	return cmd;
}
