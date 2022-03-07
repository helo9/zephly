#ifndef ZEPHLY_QUAD_SENSORS_H
#define ZEPHLY_QUAD_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

enum ZEPHLY_SENSOR_AXIS {
    ZEPHLY_X_AXIS = 0,
    ZEPHLY_Y_AXIS = 1,
    ZEPHLY_Z_AXIS = 2
};

int zephly_sensors_init();

void zephly_sensors_set_rate(float max_gyro_rates[3]);

void zephly_gyro_set_offsets(float offset_x, float offset_y, float offset_z);

int zephly_sensors_get_gyro(float measruements[3]);

#ifdef __cplusplus
}
#endif 

#endif /* ZEPHLY_QUAD_SENSORS_H */
