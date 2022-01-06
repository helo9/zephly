#include <zephyr.h>
#include "kernel.h"
#include <device.h>
#include <stdio.h>
#include <devicetree.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>

#define PWM_DEVICE1 DT_NODELABEL(pwm1)
#define PWM_DEVICE2 DT_NODELABEL(pwm2)

#define LED_NODE DT_ALIAS(userled0)
#define LED_GPIO DT_PHANDLE(LED_NODE, gpios)
#define LED_PIN	DT_GPIO_PIN(LED_NODE, gpios)

const struct device* led = DEVICE_DT_GET(LED_GPIO);

const struct device* pwm_dev1 = DEVICE_DT_GET(PWM_DEVICE1);
const struct device* pwm_dev2 = DEVICE_DT_GET(PWM_DEVICE2);

void main() {
    

    bool led_is_on = true;

    if (!device_is_ready(led)) {
        printk("LED is not ready!");
        return;
    }

    int ret = gpio_pin_configure(led, LED_PIN, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_HIGH);
    if (ret < 0) {
        printk("error!");
        return;
    }

    if (!device_is_ready(pwm_dev1) || !device_is_ready(pwm_dev2)) {
        printk("PWM is not ready!");
        return;
    }

    while (true) {
        printk("Hallo %d %d \t", ret, LED_PIN);
        ret = gpio_pin_set(led, LED_PIN, (int)led_is_on );
        led_is_on = !led_is_on;
        
        if (led_is_on) {
            ret = pwm_pin_set_usec(pwm_dev1, 4, 20000, 2000, 0);
            printk("%d\t",ret);
            ret = pwm_pin_set_usec(pwm_dev1, 3, 20000, 2000, 0);
            printk("%d\t",ret);
            ret = pwm_pin_set_usec(pwm_dev1, 2, 20000, 2000, 0);
            printk("%d\t",ret);
            ret = pwm_pin_set_usec(pwm_dev2, 1, 20000, 2000, 0);
            printk("%d\t",ret);
        } else {
            ret = pwm_pin_set_usec(pwm_dev1, 4, 20000, 1000, 0);
            printk("%d\t",ret);
            ret = pwm_pin_set_usec(pwm_dev1, 3, 20000, 1000, 0);
            printk("%d\t",ret);
            ret = pwm_pin_set_usec(pwm_dev1, 2, 20000, 1000, 0);
            printk("%d\t",ret);
            ret = pwm_pin_set_usec(pwm_dev2, 1, 20000, 1000, 0);
            printk("%d\t",ret);
        }
        printk("\n");

        k_sleep(K_MSEC(1500));
    }
}
