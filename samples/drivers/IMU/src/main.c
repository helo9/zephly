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

void main(void)
{
    struct sensor_value values[3];

	if (!device_is_ready(imu)) {
        printf("IMU is not ready!\n");

        return;
    }

	while (1) {

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

		k_sleep(K_MSEC(500));
	}
}
