#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/pwm.h>

#define PWM_PERIOD_US 20000
#define PWM_CYCLE_MIN_US 1000
#define PWM_CYCLE_MAX_US 2000
#define PWM_CYCLE_DIF (PWM_CYCLE_MAX_US - PWM_CYCLE_MIN_US)

#define PWM_DEVICE DT_NODELABEL(pwm1)

const struct device *pwm = DEVICE_DT_GET(PWM_DEVICE);

void main() {
    if (!device_is_ready(pwm)) {
        printk("PWM is not ready! Stopped application.\n");
        return;
    }

    /* variable to modify cycle time */
    uint32_t cycle_add = 0;

    while (true) {

        /* increase cycle time, but stop before we overshoot max */
        cycle_add += 50;
        if (cycle_add >= PWM_CYCLE_DIF) {
            cycle_add = 0;
        }

        /* configure the pwm pins */
        pwm_pin_set_usec(pwm, 1, PWM_PERIOD_US, 
                PWM_CYCLE_MIN_US + cycle_add, 0);
        pwm_pin_set_usec(pwm, 2, PWM_PERIOD_US,
                PWM_CYCLE_MIN_US + cycle_add, 0);
        pwm_pin_set_usec(pwm, 3, PWM_PERIOD_US,
                PWM_CYCLE_MIN_US + cycle_add, 0);
        pwm_pin_set_usec(pwm, 4, PWM_PERIOD_US,
                PWM_CYCLE_MIN_US + cycle_add, 0);

        k_sleep(K_MSEC(5));
    }
}
