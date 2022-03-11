/*
* Copyright (c) 2022 Jonathan Hahn
* SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <custom_drivers/control_output.h>

#define OUTPUT_INCREMENT 0.1f
#define OUTPUT_MIN 0.0f
#define OUTPUT_MAX 1.0f

#define CONTROL_OUTPUT_NODE DT_NODELABEL(pwmout)

float calc_test_output(float old_out);
int update_outputs(const struct device *control_out, float target);

void main() {
    const struct device *control_out = DEVICE_DT_GET(CONTROL_OUTPUT_NODE);
    float output_target = 0.0f;

    if(!device_is_ready(control_out)) {
        printk("Control output device is not ready!\n");
        return;
    }

    while (true) {
        int ret = update_outputs(control_out, output_target);
        if (ret < 0) {
            printk("Could not update outputs, failed with error %d.\n", ret);
            break;
        } else {
            printk("Updated PWM output to 0.%d\n", (int)(output_target*1000));
        }

        output_target = calc_test_output(output_target);

        k_sleep(K_SECONDS(1));
    }

    control_output_disable_motors(control_out);
}

/**
 * @brief generate stepwise increasing outputs
 * 
 * @param old_output the previous value
 * @return float the updated output value
 */
float calc_test_output(const float old_output) {
    const float tmp_out = old_output + OUTPUT_INCREMENT;

    if (tmp_out < OUTPUT_MAX) {
        return tmp_out;
    } else {
        return OUTPUT_MIN;
    }
}

/**
 * @brief write output value to all channels of output device
 * 
 * @param control_out the output device
 * @param target the target or setpoint value to write
 * @return int 0 for success, -errno otherwise
 */
int update_outputs(const struct device *control_out, const float target) {
    const float outputs[4] = {target, target, target, target};
    
    return control_output_set(control_out, outputs);
}
