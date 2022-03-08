#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <string.h>
#include "zephly_sensors.h"

#define MAX_GYRO_RATES_DEFAULT 15.0f

#define IMU_NODE DT_NODELABEL(imu)

#if CONFIG_ZEPHLY_GYRO_ORIENTATION == 0
    /* no roation */
    #define X_SIGN
    #define Y_SIGN
    #define Z_SIGN

    #define IN_X_INDEX 0
    #define IN_Y_INDEX 1
    #define IN_Z_INDEX 2
#elif CONFIG_ZEPHLY_GYRO_ORIENTATION == 1
    /* 180 degrees around roll / x axis */
    #define X_SIGN
    #define Y_SIGN -
    #define Z_SIGN -

    #define IN_X_INDEX 0
    #define IN_Y_INDEX 1
    #define IN_Z_INDEX 2
#else
    #error No gyro orientation defined
#endif

const struct device *imu = DEVICE_DT_GET(IMU_NODE);

static float max_gyro_rates[3] = {
    MAX_GYRO_RATES_DEFAULT,
    MAX_GYRO_RATES_DEFAULT,
    MAX_GYRO_RATES_DEFAULT
};

static void write_processed_measurements(float *measurements, const struct sensor_value *raw_measurements);

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

void zephly_gyro_set_offsets(float offset_x, float offset_y, float offset_z) {
    struct sensor_value val;

    sensor_value_from_double(&val, (double)offset_x);
    sensor_attr_set(imu, SENSOR_CHAN_GYRO_X, SENSOR_ATTR_OFFSET, &val);

    sensor_value_from_double(&val, (double)offset_y);
    sensor_attr_set(imu, SENSOR_CHAN_GYRO_Y, SENSOR_ATTR_OFFSET, &val);

    sensor_value_from_double(&val, (double)offset_z);
    sensor_attr_set(imu, SENSOR_CHAN_GYRO_Z, SENSOR_ATTR_OFFSET, &val);
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

    write_processed_measurements(measurements, raw_measurements);

    return 0;
}

static void write_processed_measurements(float *measurements, const struct sensor_value *raw_measurements) {
	measurements[ZEPHLY_X_AXIS] = X_SIGN sensor_value_to_double(&raw_measurements[IN_X_INDEX]) / max_gyro_rates[IN_X_INDEX];
	measurements[ZEPHLY_Y_AXIS] = Y_SIGN sensor_value_to_double(&raw_measurements[IN_Y_INDEX]) / max_gyro_rates[IN_Y_INDEX];
    measurements[ZEPHLY_Z_AXIS] = Z_SIGN sensor_value_to_double(&raw_measurements[IN_Z_INDEX]) / max_gyro_rates[IN_Z_INDEX];
}
