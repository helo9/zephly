/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <init.h>
#include <stdio.h>
#include <sys/types.h>

#include "simulator_connection_internal.h"
#include "simulator_connection_mavlink.h"

static int simulator_start_heartbeat_stream(struct simulator_data *data);

static struct simulator_data data = {
    .sim_socket = {},
    .ready = false
};

static int simulator_init() {

    int ret = simulator_socket_initialize(&data.sim_socket);

    if (ret < 0) {
        printk("socket creation failed, error %d", ret);;
        return -ret;
    }

    ret = simulator_start_heartbeat_stream(&data);

    if (ret < 0) {
        return ret;
    }

    data.ready = true;

    return 0;
}

void simulator_send_heartbeat() {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    uint16_t length = simulator_write_mavlink_hearbeat_buffer(buf);

    if (length == 0) {
        return;
    }

    simulator_socket_send(&data.sim_socket, buf, length);
}

void simulator_send_outputs(const float outputs[4]) {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    uint16_t length = simulator_write_mavlink_output_to_buffer(outputs, buf);

    if (length == 0) {
        return;
    }

    simulator_socket_send(&data.sim_socket, buf, length);
}

int simulator_update_sensor_values(float gyro[3]) {

    while (true) {
        uint8_t buf[MAVLINK_MAX_PACKET_LEN];
        int bytes_received = simulator_socket_receive(&data.sim_socket, buf, MAVLINK_MAX_PACKET_LEN);

        if (bytes_received > 0) {
            simulator_parse_message(buf, bytes_received, gyro);
        } else {
            break;
        }
    }
    return 0;
}

bool simulator_is_ready() {
    return data.ready;
}

static void simulator_heartbeat_timer_handler(struct k_timer *dummy) {
    simulator_send_heartbeat();
}

K_TIMER_DEFINE(simulator_heartbeat_timer, simulator_heartbeat_timer_handler, NULL);

static int simulator_start_heartbeat_stream(struct simulator_data *data) {
    
    k_timer_start(&simulator_heartbeat_timer, K_MSEC(10), K_MSEC(10));

    return 0;
}

SYS_INIT(simulator_init, POST_KERNEL, 64);


