#include "kernel.h"
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <zephyr.h>
#include <math.h>

#include <attitude_propagation.hpp>

#define SAMPLING_FREQUENCY 400.0

using namespace ahrs;

bool initialize_sensors(const struct device *gyro);
bool update_measurements(const struct device *gyro,
                         Vector<double, 3> &rotation_speed, double &delta_t);
void report_state(const Quaternion<double> &q, const Vector<double, 3> &w);

int main() {
  /* TODO: Find solution to get by device name */
  const struct device *gyro = DEVICE_DT_GET_ONE(st_i3g4250d);

  Quaternion<double> q = {1.0, 0.0, 0.0, 0.0};
  Vector<double, 3> w = {0.0, 0.0, 0.0};
  double delta_t = 1.0;

  if (!initialize_sensors(gyro)) {
    return -1;
  }

  while (true) {
    if (!update_measurements(gyro, w, delta_t)) {
      /* Something went wrong, try again */
      continue;
    }

    propagate_attitude<double>(q, w, delta_t/1e3);

    report_state(q, w);

    k_sleep(K_MSEC(10));
  }

  return 0;
}

bool initialize_sensors(const struct device *gyro) {
  /* ensure device is ready */
  if (!device_is_ready(gyro)) {
    return false;
  }

  /* ensure correct sampling frequency */
  struct sensor_value setting;
  sensor_value_from_double(&setting, SAMPLING_FREQUENCY);

  int rc = sensor_attr_set(gyro, SENSOR_CHAN_GYRO_XYZ,
                           SENSOR_ATTR_SAMPLING_FREQUENCY, &setting);

  if (rc != 0) {
    return false;
  }

  return true;
}

bool update_measurements(const struct device *gyro,
                         Vector<double, 3> &rotation_speed, double &delta_t) {
  static int64_t last_time = 0;

  struct sensor_value gyro_measurement[3];

  int rc = sensor_sample_fetch(gyro);

  if (rc != 0) {
    return false;
  }

  rc = sensor_channel_get(gyro, SENSOR_CHAN_GYRO_XYZ, gyro_measurement);

  if (rc != 0) {
    return false;
  }

  for (int i = 0; i < 3; i++) {
    rotation_speed.data[i] = sensor_value_to_double(&gyro_measurement[i]) / 1e3;
  }

  int64_t now = k_uptime_get();
  if ( last_time != 0 ) {
    delta_t = (now - last_time);
    last_time = now;
  } else {
    last_time = now;
    return false;
  }

  return true;
}

void report_state(const Quaternion<double> &q, const Vector<double, 3> &w) {
  /* float norm = q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z; */
  printf("%f %f %f %f", q.w, q.x, q.y, q.z);
  printf(" %lld %f %f %f \n", k_uptime_get(), w.data[0], w.data[1], w.data[2]);
}
