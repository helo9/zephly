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
}
#include <custom_drivers/rc.h>
#include <mixer/mixer.hpp>

#include <common/mavlink.h>
#include <minimal/mavlink_msg_heartbeat.h>
#include <common/mavlink_msg_hil_actuator_controls.h>

#define PORT    14560

#define RC_IN DT_NODELABEL(rc)

void main() {
    const struct device *receiver = DEVICE_DT_GET(RC_IN);
    struct Command rc_in;
    float outputs[4];

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

        /* run mixer */
		px_airmode_mix(rc_in, outputs);

        simulator_send_outputs(outputs);

        printk("out: %d %d %d %d\n", (int)(outputs[0]*100), (int)(outputs[1]*100), (int)(outputs[2]*100), (int)(outputs[3]*100));

        k_sleep(K_MSEC(10));
    }

    return;
}
