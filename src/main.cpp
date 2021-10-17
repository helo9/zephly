#include "sensors.hpp"
#include "kernel.h"
#include <device.h>
#include <stdio.h>
#include <zephyr.h>
#include <math.h>

#include <attitude_propagation.hpp>
#include "settings.hpp"

constexpr double DEG2RAD = 0.017453292519943295;

using namespace ahrs;

void report_state(const Quaternion<double> &q, const Vector<double, 3> &w, const Vector<double, 3> &a);

int main() {
  if (!initialize_sensors()) {
    printk("Initialization failed!");
    return -1;
  }

  /* initial attitude */
  Quaternion<double> q = {1.0, 0.0, 0.0, 0.0};

  while (true) {
    if (!update_measurements()) {
      /* Something went wrong, try again */
      printf("Update failed!");
      continue;
    }

    propagate_attitude<double>(q, get_rotation_speed(), get_delta_t());

    report_state(q, get_rotation_speed(), get_accelerations());

    /*auto tmp = get_mag_field();

    //printf("%f\t%f\t%f\n", tmp.data[0], tmp.data[1], tmp.data[2]);*/

    k_sleep(K_MSEC(50));
  }

  return 0;
}

void report_state(const Quaternion<double> &q, const Vector<double, 3> &w, const Vector<double, 3> &a) {
  printf("%f %f %f %f", q.w, q.x, q.y, q.z);
  printf(" %u %f %f %f", k_uptime_get_32(), w.data[0], w.data[1], w.data[2]);
  printf("%f %f %f \n", a.data[0], a.data[1], a.data[2]);
}
