/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr.h>
#include <device.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

extern "C" {
    #include <simulator_connection.h>
    #include <ratecontrol.h>
}
#include <custom_drivers/rc.h>
#include <mixer/mixer.hpp>

#include <common/mavlink.h>
#include <minimal/mavlink_msg_heartbeat.h>
#include <common/mavlink_msg_hil_actuator_controls.h>

#define NUM_AXIS 3

#define RC_IN DT_NODELABEL(rc)

static void init_ratecontrollers(struct RateControl *rate_controllers);
static void apply_control(struct RateControl *rate_controllers, const struct Command *rc_in, const float gyro[3], struct Command *rate_out);

void main() {
    const struct device *receiver = DEVICE_DT_GET(RC_IN);
    struct Command rc_in, rate_out;
    struct RateControl rate_controllers[NUM_AXIS];
    float outputs[4];
    float gyro[3];

    init_ratecontrollers(rate_controllers);

	if (!device_is_ready(receiver)) {
		printk("RC IN device not ready.\n");
		return;
	}

    if (!simulator_is_ready()) {
        printk("Simulator is not ready.\n");
        return;
    }

    while (true) {
        rc_update(receiver, &rc_in);

        simulator_update_sensor_values(gyro);

        apply_control(rate_controllers, &rc_in, gyro, &rate_out);

		px_airmode_mix(rate_out, outputs);

        simulator_send_outputs(outputs);

        printk("out: %d %d %d %d\n", (int)(outputs[0]*100), (int)(outputs[1]*100), (int)(outputs[2]*100), (int)(outputs[3]*100));

        k_sleep(K_MSEC(10));
    }

    return;
}

static void init_ratecontrollers(struct RateControl *rate_controllers) {
    for (int i=0; i<NUM_AXIS; i++) {
        ratecontrol_init(&rate_controllers[i], "roll");
        rate_controllers[i].k_p = 0.2f;
        rate_controllers[i].k_i = 0.0f;
        rate_controllers[i].k_d = 0.0f;
    }
}

static void apply_control(struct RateControl *rate_controllers, const struct Command *rc_in, const float gyro[3], struct Command *rate_out) {
    rate_out->roll  = ratecontrol_update(&rate_controllers[0], rc_in->roll, gyro[0]);
    rate_out->pitch = ratecontrol_update(&rate_controllers[0], rc_in->pitch, gyro[1]);
    rate_out->yaw   = ratecontrol_update(&rate_controllers[0], rc_in->yaw, gyro[2]);
    rate_out->thrust = rc_in->thrust;
}
