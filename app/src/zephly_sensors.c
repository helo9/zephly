#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <string.h>
#include "zephly_sensors.h"

#define MAX_GYRO_RATES_DEFAULT 15.0f

#define IMU_NODE DT_NODELABEL(imu)

const struct device *imu = DEVICE_DT_GET(IMU_NODE);

float max_gyro_rates[3] = {
    MAX_GYRO_RATES_DEFAULT,
    MAX_GYRO_RATES_DEFAULT,
    MAX_GYRO_RATES_DEFAULT
};

static void write_normalized_measurements(float *measurements, const struct sensor_value *raw_measurements);

int zephly_sensors_init() {
	if (!device_is_ready(imu)) {
		printk("Gyro device not ready!\n");
		return -EIO;
	}

    return 0;
}

void zephly_sensors_set_rate(float new_max_gyro_rates[3]) {
    memcpy(max_gyro_rates, new_max_gyro_rates, sizeof(max_gyro_rates));
}

int zephly_sensors_get_gyro(float measurements[3]) {
    int ret = 0;
    struct sensor_value raw_measurements[3];

    ret = sensor_sample_fetch(imu);

    if (ret != 0) {
        return ret;
    }

    ret = sensor_channel_get(imu, SENSOR_CHAN_GYRO_XYZ, raw_measurements);

    if (ret != 0) {
        return ret;
    }

    write_normalized_measurements(measurements, raw_measurements);

    return 0;
}

static void write_normalized_measurements(float *measurements, const struct sensor_value *raw_measurements) {
	for (int i=0; i<3; i++) {
		measurements[i] = sensor_value_to_double(&raw_measurements[i]) / max_gyro_rates[i];
	}
}
