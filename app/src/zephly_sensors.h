#ifndef ZEPHLY_QUAD_SENSORS_H
#define ZEPHLY_QUAD_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

enum ZEPHLY_SENSOR_AXIS {
    ZEPHLY_SENSOR_AXIS_PITCH = 0,
    ZEPHLY_SENSOR_AXIS_ROLL = 1,
    ZEPHLY_SENSOR_AXIS_YAW = 2
};

int zephly_sensors_init();

void zephly_sensors_set_rate(float max_gyro_rates[3]);

int zephly_sensors_get_gyro(float measruements[3]);

#ifdef __cplusplus
}
#endif 

#endif /* ZEPHLY_QUAD_SENSORS_H */
