/*
* Copyright (c) 2022 Jonathan Hahn
* SPDX-License-Identifier: Apache-2.0
*/

#include <drivers/pwm.h>
#include <device.h>
#include <custom_drivers/control_output.h>

#define DT_DRV_COMPAT zephly_pwmout

#define PWM_OUTPUT_CHANNELS 4

#define NODE_OKAY(node) DT_NODE_HAS_STATUS(DT_NODELABEL(node), okay)
#define GET_PWM(node) DEVICE_DT_GET(DT_NODELABEL(node))
#define GET_PWM_NUM_PINS(node) 0

#define PWM_PERIOD CONFIG_CONTROL_OUTPUT_PWM_PERIOD /* 50 Hz update rate */
#define PWM_MIN_DUTY CONFIG_CONTROL_OUTPUT_PWM_MIN_DUTY /* 1ms is minimal duty cycle */
#define PWM_MAX_DUTY CONFIG_CONTROL_OUTPUT_PWM_MAX_DUTY /* 2ms is maximum duty cycle */

struct output_pwm_data {};
struct output_pwm_config {
    const struct pwm_dt_spec pwms[PWM_OUTPUT_CHANNELS];
};

static int output_pwm_init(const struct device *dev) {
    const struct output_pwm_config *config = (const struct output_pwm_config*)dev->config;

    for (int i=0; i<4; i++) {
        if (!device_is_ready(config->pwms[i].dev)) {
            return -EIO;
        }
    }

    return 0;
}

static uint32_t get_pulse(float setpoint) {
    return PWM_MIN_DUTY + (uint32_t)( (PWM_MAX_DUTY - PWM_MIN_DUTY) * setpoint);
}

static int output_pwm_set(const struct device *dev, const float outputs[4]) {
    const struct output_pwm_config *config = (const struct output_pwm_config*)dev->config;

    for (int i=0; i<4; i++) {
        const struct pwm_dt_spec *tmp = &config->pwms[i];
        const uint32_t pulse = get_pulse(outputs[i]);
        int ret = pwm_set(tmp->dev, tmp->channel, PWM_USEC(PWM_PERIOD), PWM_USEC(pulse), 0);

        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

const struct control_output_api g_api = {
    .set = output_pwm_set
};

#if DT_NODE_HAS_STATUS(DT_NODELABEL(pwmout), okay)

struct output_pwm_data data;
const struct output_pwm_config config = {
    .pwms = {
        PWM_DT_SPEC_GET_BY_IDX(DT_NODELABEL(pwmout), 0),
        PWM_DT_SPEC_GET_BY_IDX(DT_NODELABEL(pwmout), 1),
        PWM_DT_SPEC_GET_BY_IDX(DT_NODELABEL(pwmout), 2),
        PWM_DT_SPEC_GET_BY_IDX(DT_NODELABEL(pwmout), 3)
    }
};

DEVICE_DT_INST_DEFINE(0, &output_pwm_init, NULL,
		      &data, &config,
		      POST_KERNEL, CONFIG_CONTROL_OUTPUT_PWM_INIT_PRIORITY,
		      &g_api);

#endif
