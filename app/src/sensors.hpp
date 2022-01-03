#ifndef SENSORS_HPP
#define SENSORS_HPP

#include <attitude_estimation.hpp>
#include <device.h>
#include <zephyr.h>

union Sensors {
    /* TODO: unnamed struct in union is not covered by c++ standard.. */
    struct {
        const struct device* gyro;
        const struct device* mag;
        const struct device* accel;
    };
    const struct device* array[3];
};

struct Measurements {
    AttitudeEstimation::Vector<double, 3> rotation_speed;
    AttitudeEstimation::Vector<double, 3> mag_field;
    AttitudeEstimation::Vector<double, 3> accelerations;
    double delta_t;
};

bool initialize_sensors();

bool update_measurements();

const AttitudeEstimation::Vector<double, 3>& get_rotation_speed();
const AttitudeEstimation::Vector<double, 3>& get_mag_field();
const AttitudeEstimation::Vector<double, 3>& get_accelerations();
const double& get_delta_t();

#endif /* SENSORS_HPP */
