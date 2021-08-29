#pragma once

namespace ahrs {

template <typename T> struct Quaternion {
  T w;
  T x;
  T y;
  T z;
};

template <typename T, int n> struct Vector { T data[n]; };

template <typename T>
void propagate_attitude(Quaternion<T> &attitude, const Vector<T, 3> &rotation_speed,
                        const T delta_t) {

  const auto [qw, qx, qy, qz] = attitude;
  const auto &[wx, wy, wz] = rotation_speed.data;

  /* Do the actual propagation, see Graf (2007) - Quaternions And Dynamics */

  attitude.w -= delta_t * (wx * qx + wy * qy + wz * qz) / 2.0;
  attitude.x += delta_t * (wx * qw - wy * qz + wz * qy) / 2;
  attitude.y += delta_t * (wx * qz + wy * qw - wz * qx) / 2;
  attitude.z += delta_t * (-wx * qy + wy * qx + wz * qw) / 2;

  /* Normalize the quaternion again to get attitude quaternion */

  const auto norm = attitude.w * attitude.w + attitude.x * attitude.x +
                    attitude.y * attitude.y + attitude.z * attitude.z;

  attitude.w /= norm;
  attitude.x /= norm;
  attitude.y /= norm;
  attitude.z /= norm;
}

}
