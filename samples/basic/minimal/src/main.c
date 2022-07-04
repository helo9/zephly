#include <zephyr.h>
#include <drivers/gpio.h>

#define SLEEP_TIME_MS   1000

#define LED_NODE DT_ALIAS(led1)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

void main() {
	int ret;

	if (!device_is_ready(led.port)) {
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_LINE_OPEN_DRAIN|GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

    while (true) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return;
		}
		k_msleep(SLEEP_TIME_MS);
    }
    
}
