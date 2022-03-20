/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kernel.h"
#include <zephyr.h>
#include <device.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <custom_drivers/rc.h>
#include <mixer/mixer.hpp>

#include <common/mavlink.h>
#include <minimal/mavlink_msg_heartbeat.h>
#include <common/mavlink_msg_hil_actuator_controls.h>

#define PORT    14560
#define MAXLINE 1024

#define RC_IN DT_NODELABEL(rc)

void set_server_address(struct sockaddr_in *addr);
int fill_actuator_output_buffer(uint8_t *buf, uint16_t *len, float outputs[4]);
int fill_hearbeat_buffer(uint8_t *buffer, uint16_t *len);

void main() {
    const struct device *receiver = DEVICE_DT_GET(RC_IN);
    struct Command rc_in;
    int sockfd;
    struct sockaddr_in servaddr;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN], buf2[MAVLINK_MAX_PACKET_LEN];
    uint16_t length, length2;
    float outputs[4];

    printk("Hello my dear friend :)\n");

    // check for rc device
	if (!device_is_ready(receiver)) {
		printk("RC IN device not ready.\n");
		return;
	}

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return;
    }

    set_server_address(&servaddr);
    fill_hearbeat_buffer(buf2, &length2);

    while (true) {
        rc_update(receiver, &rc_in);

        /*printk("Got roll: %d, pitch: %d, thrust: %d, yaw: %d, armed: %d\n", 
                (int)(rc_in.roll*100), (int)(rc_in.pitch*100), 
                (int)(rc_in.thrust*100), (int)(rc_in.yaw*100),
                (int)(rc_in.armed));*/

        /* run mixer */
		px_airmode_mix(rc_in, outputs);

        fill_actuator_output_buffer(buf, &length, outputs);

        printk("out: %d %d %d %d\n", (int)(outputs[0]*100), (int)(outputs[1]*100), (int)(outputs[2]*100), (int)(outputs[3]*100));

    	sendto(sockfd, (const char *)buf, length,
            MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
		
		sendto(sockfd, (const char *)buf2,  length2,
            MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
        k_sleep(K_MSEC(10));
    }

    return;
}

void set_server_address(struct sockaddr_in *addr) {
	memset(addr, 0, sizeof(*addr));

	// Filling server information
    addr->sin_family = AF_INET;
    addr->sin_port = htons(PORT);
    addr->sin_addr.s_addr = inet_addr("127.0.0.1");
}

int fill_actuator_output_buffer(uint8_t *buf, uint16_t *len, float outputs[4]) {
	mavlink_message_t msg;
	mavlink_hil_actuator_controls_t actuator_controls = {};

	actuator_controls.controls[0] = outputs[3];
    actuator_controls.controls[1] = outputs[1];
    actuator_controls.controls[2] = outputs[0];
    actuator_controls.controls[3] = outputs[2];
	
	actuator_controls.mode = MAV_MODE_FLAG_SAFETY_ARMED;

	int ret = mavlink_msg_hil_actuator_controls_encode(1, 1, &msg, &actuator_controls);
    if (ret < 0) {
        return ret;
    }

	*len = mavlink_msg_to_send_buffer(buf, &msg);

    return 0;
}

int fill_hearbeat_buffer(uint8_t *buffer, uint16_t *len) {
	mavlink_message_t msg;
	mavlink_heartbeat_t heartbeat = {};

	heartbeat.base_mode = MAV_MODE_FLAG_SAFETY_ARMED;

	int ret = mavlink_msg_heartbeat_encode(1, 1, &msg, &heartbeat);
    if (ret < 0) {
        return ret;
    }
	
    *len = mavlink_msg_to_send_buffer(buffer, &msg);

    return 0;
}
