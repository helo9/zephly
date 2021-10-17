#include "sensors.hpp"
#include <drivers/sensor.h>

constexpr double SAMPLING_FREQUENCY = 400.0;
constexpr double DEG2RAD = 0.017453292519943295;

static Sensors sensors {
    DEVICE_DT_GET_ONE(st_i3g4250d),
    DEVICE_DT_GET_ONE(st_lsm303agr_magn),
    DEVICE_DT_GET_ONE(st_lsm303agr_accel)
};

static Measurements measurements {};

bool initialize_sensors() {
  /* ensure device is ready */
  if (!device_is_ready(sensors.gyro) || !device_is_ready(sensors.mag) || !device_is_ready(sensors.accel)) {
    return false;
  }

  /* configure sampling frequency of gyro */
  struct sensor_value setting;
  sensor_value_from_double(&setting, SAMPLING_FREQUENCY);

  int rc = sensor_attr_set(sensors.gyro, SENSOR_CHAN_GYRO_XYZ,
                           SENSOR_ATTR_SAMPLING_FREQUENCY, &setting);

  if (rc != 0) {
    return false;
  }

  return true;
}

bool update_measurements() {
  static int64_t last_time = 0;

  int rc;

  struct sensor_value raw_measurements[3];

  for (int i=0; i<3; i++) {
    rc = sensor_sample_fetch(sensors.array[i]);
    if (rc != 0) {
      return false;
    }
  }

  rc = sensor_channel_get(sensors.gyro, SENSOR_CHAN_GYRO_XYZ, raw_measurements);
  if (rc != 0) {
    return false;
  }

  for (int i = 0; i < 3; i++) {
    measurements.rotation_speed.data[i] = sensor_value_to_double(&raw_measurements[i]) * DEG2RAD;
  }

  rc = sensor_channel_get(sensors.mag, SENSOR_CHAN_MAGN_XYZ, raw_measurements);
  if (rc != 0) {
    return false;
  }

  for (int i = 0; i < 3; i++) {
    measurements.mag_field.data[i] = sensor_value_to_double(&raw_measurements[i]);
  }

  rc = sensor_channel_get(sensors.accel, SENSOR_CHAN_ACCEL_XYZ, raw_measurements);
  if (rc != 0) {
    return false;
  }

  for (int i = 0; i < 3; i++) {
    measurements.accelerations.data[i] = sensor_value_to_double(&raw_measurements[i]);
  }

  int64_t now = k_uptime_get();
  if ( last_time != 0 ) {
    measurements.delta_t = (now - last_time) / 1e3;
    last_time = now;
  } else {
    last_time = now;
    return false;
  }

  return true;
}

const ahrs::Vector<double, 3>& get_rotation_speed() {
    return measurements.rotation_speed;
}

const ahrs::Vector<double, 3>& get_mag_field() {
    return measurements.mag_field;
}

const ahrs::Vector<double, 3>& get_accelerations() {
  return measurements.accelerations;
}

const double& get_delta_t() {
    return measurements.delta_t;
}
