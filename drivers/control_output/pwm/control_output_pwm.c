/*
* Copyright (c) 2022 Jonathan Hahn
* SPDX-License-Identifier: Apache-2.0
*/

#include <drivers/pwm.h>
#include <device.h>
#include <custom_drivers/control_output.h>

#define DT_DRV_COMPAT zephly_pwmout

#define NODE_OKAY(node) DT_NODE_HAS_STATUS(DT_NODELABEL(node), okay)
#define GET_PWM(node) DEVICE_DT_GET(DT_NODELABEL(node))
#define GET_PWM_NUM_PINS(node) 0

#define PWM_PERIOD 20000 /* 50 Hz update rate */
#define PWM_MIN_DUTY 1000 /* 1ms is minimal duty cycle */
#define PWM_MAX_DUTY 2000 /* 2ms is maximum duty cycle */

struct output_pwm_data {};
struct output_pwm_config {
    const struct device *pwm_dev[2];
};

static int output_pwm_init(const struct device *dev) {
    const struct output_pwm_config *config = (const struct output_pwm_config*)dev->config;

    if (!device_is_ready(config->pwm_dev[0]) && !device_is_ready(config->pwm_dev[1]) ) {
        return -EIO;
    }

    return 0;
}

static uint32_t get_pulse(float setpoint) {
    return PWM_MIN_DUTY + (uint32_t)( (PWM_MAX_DUTY - PWM_MIN_DUTY) * setpoint);
}

static int output_pwm_set(const struct device *dev, const float outputs[4]) {
    const struct output_pwm_config *config = (const struct output_pwm_config*)dev->config;

    int ret = pwm_pin_set_usec(config->pwm_dev[0], 4, PWM_PERIOD, get_pulse(outputs[0]), 0);
    if (ret < 0) {
        return ret;
    }

    ret = pwm_pin_set_usec(config->pwm_dev[0], 3, PWM_PERIOD, get_pulse(outputs[1]), 0);
    if (ret < 0) {
        return ret;
    }

    ret = pwm_pin_set_usec(config->pwm_dev[0], 2, PWM_PERIOD, get_pulse(outputs[2]), 0);
    if (ret < 0) {
        return ret;
    }

    ret = pwm_pin_set_usec(config->pwm_dev[1], 1, PWM_PERIOD, get_pulse(outputs[3]), 0);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

const struct control_output_api api = {
    .set = output_pwm_set
};

struct output_pwm_data data;
const struct output_pwm_config config = {
    .pwm_dev = {
        GET_PWM(pwm1),
        GET_PWM(pwm2)
    }
};

DEVICE_DT_INST_DEFINE(0, &output_pwm_init, NULL,
		      &data, &config,
		      POST_KERNEL, CONFIG_CONTROL_OUTPUT_PWM_INIT_PRIORITY,
		      &api);
