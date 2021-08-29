#include "attitude_propagation.hpp"

template<typename T>
void propagate_attitude(Quaternion<T>& last_attitude, Vector<T, 3> rotation_speed, T delta_t) {

    auto [q0, q1, q2, q3] = last_attitude;
}
