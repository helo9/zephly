/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <stdio.h>

const struct device *imu = DEVICE_DT_GET(DT_NODELABEL(imu));

void collect_imu_values(struct k_work *work) {

    struct sensor_value values[3];

    int ret = sensor_sample_fetch(imu);
    if (ret == 0) {
        printf("Succesfully fetched data :)\n");
    } else {
        printf("Something went wrong during fetching (error %d) :(\n", ret);
        return;
    }

    ret = sensor_channel_get(imu, SENSOR_CHAN_GYRO_XYZ, values);
    if (ret == 0) {
        printf("Succesfully read gyro values: %f %f %f\n",
            sensor_value_to_double(&values[0]), 
            sensor_value_to_double(&values[1]),
            sensor_value_to_double(&values[2]));
    }

    ret = sensor_channel_get(imu, SENSOR_CHAN_ACCEL_XYZ, values);
    if (ret == 0) {
        printf("Succesfully read accel values: %f %f %f\n",
            sensor_value_to_double(&values[0]), 
            sensor_value_to_double(&values[1]),
            sensor_value_to_double(&values[2]));
    }
}

K_WORK_DEFINE(imu_work, collect_imu_values);


void measurement_timer_handler(struct k_timer *dummy)
{
    k_work_submit(&imu_work);
}

K_TIMER_DEFINE(measurement_timer, measurement_timer_handler, NULL);

void main(void)
{

	if (!device_is_ready(imu)) {
        printf("IMU is not ready!\n");

        return;
    }

    k_timer_start(&measurement_timer, K_MSEC(500), K_MSEC(500));
}
