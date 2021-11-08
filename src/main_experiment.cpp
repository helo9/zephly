#include <zephyr.h>
#include "kernel.h"
#include <device.h>
#include <stdio.h>
#include <devicetree.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>

#define PWM_DEVICE DT_NODELABEL(pwm1)
#define PWM_CHANNEL DT_PWMS_CHANNEL_BY_IDX(PWM_DEVICE, 0)

#define LED_NODE DT_ALIAS(userled0)
#define LED_GPIO DT_PHANDLE(LED_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED_NODE, gpios)

const struct device* led = DEVICE_DT_GET(LED_GPIO);

void main() {
    //const struct device* pwm_dev = DEVICE_DT_GET(PWM_DEVICE);

    bool led_is_on = true;

    if (!device_is_ready(led)) {
        printk("LED is not ready!");
        return;
    }

    int ret = gpio_pin_configure(led, 3, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_HIGH);
    if (ret < 0) {
        return;
    }

    /*if (!device_is_ready(pwm_dev)) {
        printk("PWM is not ready!");
        return;
    }*/

    while (true) {
        printk("Hallo %d %d \t", ret, PIN);
        ret = gpio_pin_set(led, PIN, (int)led_is_on );
        led_is_on = !led_is_on;
        /*int err = 0;
        err |= pwm_pin_set_usec(pwm_dev, 1, 20000, 2000, 0);
        err |= pwm_pin_set_usec(pwm_dev, 2, 20000, 2000, 0);
        err |= pwm_pin_set_usec(pwm_dev, 3, 20000, 2000, 0);
        err |= pwm_pin_set_usec(pwm_dev, 4, 20000, 2000, 0);

        printk("%d\n", err);*/

        k_sleep(K_MSEC(1500));
    }
}
