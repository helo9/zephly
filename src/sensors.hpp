#ifndef SENSORS_HPP
#define SENSORS_HPP

#include <attitude_propagation.hpp>
#include <device.h>
#include <zephyr.h>

union Sensors {
    /* TODO: unnamed struct in union is not covered by c++ standard.. */
    struct {
        const struct device* gyro;
        const struct device* mag;
    };
    const struct device* array[2];
};

struct Measurements {
    ahrs::Vector<double, 3> rotation_speed;
    ahrs::Vector<double, 3> mag_field;
    double delta_t;
};

bool initialize_sensors();

bool update_measurements();

const ahrs::Vector<double, 3>& get_rotation_speed();
const ahrs::Vector<double, 3>& get_mag_field();
const double& get_delta_t();

#endif /* SENSORS_HPP */
