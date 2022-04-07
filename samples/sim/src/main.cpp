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

#define PORT    14560

#define RC_IN DT_NODELABEL(rc)

static void reduce_to_roll_axis(struct Command *rc_in);

void main() {
    const struct device *receiver = DEVICE_DT_GET(RC_IN);
    struct Command rc_in;
    struct RateControl roll_rate_control;
    float outputs[4];
    float gyro[3];

    ratecontrol_init(&roll_rate_control, "roll");
    roll_rate_control.k_p = 0.2f;
    roll_rate_control.k_i = 0.0f;
    roll_rate_control.k_d = 0.0f;

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

        reduce_to_roll_axis(&rc_in);

        float corrected = ratecontrol_update(&roll_rate_control, rc_in.roll, gyro[0]);

        printk("%d - %d - %d\n", (int)(1000*rc_in.roll), (int)(1000*gyro[0]), (int)(1000*corrected));

        rc_in.roll = corrected;

		px_airmode_mix(rc_in, outputs);

        simulator_send_outputs(outputs);

        //printk("out: %d %d %d %d\n", (int)(outputs[0]*100), (int)(outputs[1]*100), (int)(outputs[2]*100), (int)(outputs[3]*100));

        k_sleep(K_MSEC(10));
    }

    return;
}


static void reduce_to_roll_axis(struct Command *rc_in) {
    rc_in->pitch = 0.0f;
    rc_in->yaw = 0.0f;
}
