#include <zephyr.h>
#include <drivers/gpio.h>

#define SLEEP_TIME_MS   2000

#define LED_NODE0 DT_ALIAS(led0)
#define LED_NODE1 DT_ALIAS(led1)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED_NODE0, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED_NODE1, gpios);

void main() {

	printk("Starting Minimal Example\n");

	int ret;

	if (!device_is_ready(led0.port) || !device_is_ready(led1.port)) {
		return;
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_LINE_OPEN_DRAIN|GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_LINE_OPEN_DRAIN|GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

	ret = gpio_pin_toggle_dt(&led0);
	if (ret < 0) {
		return;
	}

    while (true) {
		printk("Toggle LED :)\n");
		ret = gpio_pin_toggle_dt(&led0);
		if (ret < 0) {
			return;
		}

		ret = gpio_pin_toggle_dt(&led1);
		if (ret < 0) {
			return;
		}
		k_msleep(SLEEP_TIME_MS);
    }
    
}
