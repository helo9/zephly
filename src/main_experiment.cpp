#include "kernel.h"
#include <device.h>
#include <stdio.h>
#include <zephyr.h>
#include <drivers/pwm.h>

#define PWM_DEVICE DT_NODELABEL(pwm1)
//#define PWM_CHANNEL DT_PWMS_CHANNEL_BY_IDX(PWM_DEVICE, 0)

void main() {
    const struct device* pwm_dev = DEVICE_DT_GET(PWM_DEVICE);

    if (!device_is_ready(pwm_dev)) {
        printk("PWM is not ready!");
        return;
    }

    while (true) {
        printk("Hallo\t");
        int err = 0;
        err |= pwm_pin_set_usec(pwm_dev, 1, 20000, 2000, 0);
        err |= pwm_pin_set_usec(pwm_dev, 2, 20000, 2000, 0);
        err |= pwm_pin_set_usec(pwm_dev, 3, 20000, 2000, 0);
        err |= pwm_pin_set_usec(pwm_dev, 4, 20000, 2000, 0);

        printk("%d\n", err);

        k_sleep(K_MSEC(500));
    }
}
