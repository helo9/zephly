#pragma once

#include <stdlib.h>
#include <math.h>

namespace AttitudeEstimation {

template <typename T>
static T sqrt(T value)
{
	int i;
	T sqrt = value / 3;

	if (value <= 0) {
		return 0;
	}

	for (i = 0; i < 12; i++) {
		sqrt = (sqrt + value / sqrt) / 2;
	}

	return sqrt;
}

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
  attitude.x += delta_t * (wx * qw - wy * qz + wz * qy) / 2.0;
  attitude.y += delta_t * (wx * qz + wy * qw - wz * qx) / 2.0;
  attitude.z += delta_t * (-wx * qy + wy * qx + wz * qw) / 2.0;

  /* Normalize the quaternion again to get an attitude quaternion */

  const T norm = sqrt(attitude.w * attitude.w + attitude.x * attitude.x +
                    attitude.y * attitude.y + attitude.z * attitude.z);

  attitude.w /= norm;
  attitude.x /= norm;
  attitude.y /= norm;
  attitude.z /= norm;
}

}
