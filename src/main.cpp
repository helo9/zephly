#include "sensors.hpp"
#include "kernel.h"
#include <device.h>
#include <stdio.h>
#include <zephyr.h>
#include <math.h>

#include <attitude_propagation.hpp>

constexpr double DEG2RAD = 0.017453292519943295;

using namespace ahrs;

void report_state(const Quaternion<double> &q, const Vector<double, 3> &w);

int main() {
  if (!initialize_sensors()) {
    return -1;
  }

  /* initial attitude */
  Quaternion<double> q = {1.0, 0.0, 0.0, 0.0};

  while (true) {
    if (!update_measurements()) {
      /* Something went wrong, try again */
      continue;
    }

    propagate_attitude<double>(q, get_rotation_speed(), get_delta_t());

    report_state(q, get_rotation_speed());

    //auto tmp = get_mag_field();

    //printf("%f\t%f\t%f\n", tmp.data[0], tmp.data[1], tmp.data[2]);

    k_sleep(K_MSEC(10));
  }

  return 0;
}

void report_state(const Quaternion<double> &q, const Vector<double, 3> &w) {
  /* float norm = q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z; */
  printf("%f %f %f %f", q.w, q.x, q.y, q.z);
  printf(" %lld %f %f %f \n", k_uptime_get(), w.data[0], w.data[1], w.data[2]);
}
