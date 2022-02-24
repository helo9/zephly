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

#define RC_IN DT_NODELABEL(rc)
#define IMU_NODE DT_NODELABEL(imu)

const struct device *rc = DEVICE_DT_GET(RC_IN);
const struct device *imu = DEVICE_DT_GET(IMU_NODE);

/* initial pwm values */
static float outputs[] = {0.0f, 0.0f, 0.0f, 0.0f};

/* storage for rc inputs */
static struct Command rc_val = {};

/* setpoints */
struct sensor_value gyro[3];
float roll_measurement = 0.0f;

static struct RateControl ratecontrols[3];

static void configure_ratecontrol(struct RateControl *ratecontrols);
inline struct Command calculate_setpoint(const struct Command &rc_cmd);
inline struct Command run_ratecontrol(struct RateControl *ratecontrol, const struct Command &setpoints);

int main() {
	printk("Starting initialization.\n");

	ratecontrol_init(&ratecontrols[0], "roll");
	ratecontrol_init(&ratecontrols[1], "yaw");
	ratecontrol_init(&ratecontrols[2], "pitch");

	configure_ratecontrol(ratecontrols);

	if (!device_is_ready(rc)) {
		printk("RC IN device not ready.");
		return -EIO;
	}

	if (!device_is_ready(imu)) {
		printk("Gyro device not ready!\n");
		return -EIO;
	}

	auto &pwm = PWM::init();

	/* write initial pwm values */
	pwm.write(outputs);

	printk("Initialization complete.\nRunning!\n");

	while (true) {
		/* update rc */
		rc_update(rc, &rc_val);

		/* get new sensor measurements */
		sensor_sample_fetch(imu);
		sensor_channel_get(imu, SENSOR_CHAN_GYRO_XYZ, gyro);

		/* get setpoint from rc_val */
		const auto setpoint = calculate_setpoint(rc_val);
		const auto commands = run_ratecontrol(ratecontrols, setpoint);
		
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
	static constexpr float MAX_RATE = 50.0f;

	struct Command setpoint;
	setpoint.pitch  = rc_cmd.pitch * MAX_RATE;
	setpoint.roll 	= rc_cmd.roll * MAX_RATE;
	setpoint.yaw	= rc_cmd.yaw * MAX_RATE;

	return setpoint;
}

inline struct Command run_ratecontrol(struct RateControl *ratecontrols, const struct Command &setpoints) {
	struct Command cmd;
	cmd.pitch 	= ratecontrol_update(&ratecontrols[0], setpoints.pitch, 0.0f);
	cmd.roll	= ratecontrol_update(&ratecontrols[1], setpoints.roll, 0.0f);
	cmd.yaw 	= ratecontrol_update(&ratecontrols[2], setpoints.yaw, 0.0f);

	return cmd;
}
