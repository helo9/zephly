/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include "simulator_connection_mavlink.h"

#include <common/mavlink.h>
#include <mavlink_helpers.h>
#include <minimal/mavlink_msg_heartbeat.h>
#include <common/mavlink_msg_hil_actuator_controls.h>

uint16_t simulator_write_mavlink_hearbeat_buffer(uint8_t *buffer) {
	mavlink_message_t msg;
	mavlink_heartbeat_t heartbeat = {};

	heartbeat.base_mode = MAV_MODE_FLAG_SAFETY_ARMED;

	int ret = mavlink_msg_heartbeat_encode(1, 1, &msg, &heartbeat);
    if (ret < 0) {
        return ret;
    }
	
    return mavlink_msg_to_send_buffer(buffer, &msg);
}

uint16_t simulator_write_mavlink_output_to_buffer(const float outputs[4], uint8_t *buffer) {
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

int simulator_parse_message(uint8_t *buffer, size_t buffer_len, float gyro[3]) {
    mavlink_status_t status;
    mavlink_message_t msg;
    uint8_t ret;

    for (size_t i=0; i<buffer_len; i++) {
        ret = mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &msg, &status);
        if (ret == 1) {
            break;
        }
    }

    if (ret != 1 || msg.msgid != MAVLINK_MSG_ID_HIL_SENSOR) {
        return -EINVAL;
    }

    mavlink_hil_sensor_t hil_sensor;
    mavlink_msg_hil_sensor_decode(&msg, &hil_sensor);

    gyro[0] = hil_sensor.xgyro;
    gyro[1] = hil_sensor.ygyro;
    gyro[2] = hil_sensor.zgyro;

    return 0;
}
