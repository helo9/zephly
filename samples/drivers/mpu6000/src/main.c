#include <zephyr.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>


#define LED_NODE DT_ALIAS(userled0)
#define LED_GPIO DT_PHANDLE(LED_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED_NODE, gpios)

#define MPU6000 DT_NODELABEL(imu)

const struct device *led = DEVICE_DT_GET(LED_GPIO);
const struct device *imu = DEVICE_DT_GET(MPU6000);

void main() {

    bool led_is_on = true;

    if (!device_is_ready(led)) {
        printk("LED is not ready!");
        return;
    }

    int ret = gpio_pin_configure(led, 3, GPIO_OUTPUT_HIGH | GPIO_ACTIVE_HIGH);
    if (ret < 0) {
        return;
    }

    if (!device_is_ready(imu)) {
        printk("MPU6000 is not ready!");
        return;
    }

    while (true) {
        printk("Noch da :)\n");

        ret = gpio_pin_set(led, PIN, (int)led_is_on );
        led_is_on = !led_is_on;

        //ret = request_wai(spi1);
        ret = sensor_sample_fetch(imu);

        if (ret != 0) {
            printk("Probleme probleme probleme.. (%d)\n", ret);
            ret = 0;
        }
        
        k_sleep(K_MSEC(1000));
    }
}
