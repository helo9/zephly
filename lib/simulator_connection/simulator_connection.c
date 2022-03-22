/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "simulator_connection.h"
#include <zephyr.h>
#include <init.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <simulator_connection.h>

#include <common/mavlink.h>
#include <minimal/mavlink_msg_heartbeat.h>
#include <common/mavlink_msg_hil_actuator_controls.h>

#define SIMULATOR_IP "127.0.0.1"
#define SIMULATOR_PORT    14560

struct simulator_data {
    int sockfd;
    struct sockaddr_in servaddr;
    bool ready;
};

static struct simulator_data data = {
    .sockfd = 0,
    .servaddr = {},
    .ready = false
};

uint16_t simulator_write_hearbeat_buffer(uint8_t *buffer) {
	mavlink_message_t msg;
	mavlink_heartbeat_t heartbeat = {};

	heartbeat.base_mode = MAV_MODE_FLAG_SAFETY_ARMED;

	int ret = mavlink_msg_heartbeat_encode(1, 1, &msg, &heartbeat);
    if (ret < 0) {
        return ret;
    }
	
    return mavlink_msg_to_send_buffer(buffer, &msg);
}

void simulator_send_heartbeat() {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    uint16_t length = simulator_write_hearbeat_buffer(buf);

    if (length <= 0) {
        return;
    }

    sendto(data.sockfd, (const char*)buf, length,
        MSG_CONFIRM, (const struct sockaddr *)&data.servaddr,
        sizeof(data.servaddr));
}

uint16_t simulator_write_mavlink_output_to_buffer(float outputs[4], uint8_t *buffer) {
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

	return mavlink_msg_to_send_buffer(buffer, &msg);
}

void simulator_send_outputs(float outputs[4]) {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];


    uint16_t length = simulator_write_mavlink_output_to_buffer(outputs, buf);

    if (length <= 0) {
        return;
    }

    sendto(data.sockfd, (const char*)buf, length,
        MSG_CONFIRM, (const struct sockaddr *)&data.servaddr,
        sizeof(data.servaddr));
}

static void simulator_initialize_sockaddr(struct sockaddr_in *addr) {
    memset(addr, 0, sizeof(*addr));

	/* Filling server information */
    addr->sin_family = AF_INET;
    addr->sin_port = htons(SIMULATOR_PORT);
    addr->sin_addr.s_addr = inet_addr(SIMULATOR_IP);
}

static int simulator_start_heartbeat_stream(struct simulator_data *data) {
    return 0;
}

static void simulator_heartbeat_timer_handler(struct k_timer *dummy) {
    simulator_send_heartbeat();
}

K_TIMER_DEFINE(simulator_heartbeat_timer, simulator_heartbeat_timer_handler, NULL);

static int simulator_init() {
    printk("Called simulator__init :) \n");

    simulator_initialize_sockaddr(&data.servaddr);

    data.sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (data.sockfd < 0) {
        perror("socket creation failed");
        return -EIO;
    }

    int ret = simulator_start_heartbeat_stream(&data);

    if (ret < 0) {
        return ret;
    }

    k_timer_start(&simulator_heartbeat_timer, K_SECONDS(1), K_SECONDS(1));

    data.ready = true;

    return 0;
}

bool simulator_is_ready() {
    return data.ready;
}

SYS_INIT(simulator_init, POST_KERNEL, 64);


