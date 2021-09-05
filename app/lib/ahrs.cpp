
#include "attitude_propagation.hpp"

extern "C" {

#include "ahrs.h"

void propagate_attitudef(float *qw, float *qx, float *qy, float *qz,
                         const float wx, const float wy, const float wz,
                         const float delta_t) {
  ahrs::Quaternion<float> q {*qw, *qx, *qy, *qz};
  ahrs::Vector<float, 3> w {wx, wy, wz};

  ahrs::propagate_attitude<float>(q, w, delta_t);

  *qw = q.w;
  *qx = q.x;
  *qy = q.y;
  *qz = q.z;
}

void propagate_attituded(double *qw, double *qx, double *qy, double *qz,
                         const double wx, const double wy, const double wz,
                         const double delta_t) {
  ahrs::Quaternion<double> q {*qw, *qx, *qy, *qz};
  ahrs::Vector<double, 3> w {wx, wy, wz};

  ahrs::propagate_attitude<double>(q, w, delta_t);

  *qw = q.w;
  *qx = q.x;
  *qy = q.y;
  *qz = q.z;
}

}
