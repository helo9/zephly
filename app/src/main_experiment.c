#include <zephyr.h>
#include "kernel.h"
#include <device.h>
#include <stdio.h>
#include <devicetree.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>

#define PWM_DEVICE1 DT_NODELABEL(pwm1)
#define PWM_DEVICE2 DT_NODELABEL(pwm2)

#define LED_NODE DT_ALIAS(userled0)
#define LED_GPIO DT_PHANDLE(LED_NODE, gpios)
#define LED_PIN	DT_GPIO_PIN(LED_NODE, gpios)

#define SPI1 DT_NODELABEL(spi1)
#define CS_GPIO DT_NODELABEL(gpioa)

const struct device* led = DEVICE_DT_GET(LED_GPIO);

const struct device* pwm_dev1 = DEVICE_DT_GET(PWM_DEVICE1);
const struct device* pwm_dev2 = DEVICE_DT_GET(PWM_DEVICE2);

const struct device * spi1 = DEVICE_DT_GET(SPI1);

const struct spi_cs_control cs_ctrl = 
{
	.gpio_dev = DEVICE_DT_GET(CS_GPIO),
	.delay = 0,
	.gpio_pin = 4,
	.gpio_dt_flags = GPIO_ACTIVE_LOW
};
const struct spi_config spi1_config = {
	.frequency = 562500,
	.operation =SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
	.cs = &cs_ctrl,
};

int read_wai() {
	int err;
	uint8_t tx_bytes = 0x75;
	uint8_t data;
	const struct spi_buf tx_buf = {
		.buf = &tx_bytes,
		.len = 1,
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1,
	};

	const struct spi_buf rx_bufs[] = {
		{
			.buf = NULL,
			.len = 1,
		},
		{
			.buf = &data,
			.len = 1,
		}
	};
	const struct spi_buf_set rx = {
		.buffers = rx_bufs,
		.count = 2,
	};

	err = spi_transceive(spi1, &spi1_config, &tx, &rx);

	if ( err < 0 ) {
		return err;
	} else {
		return data;
	}
}

void main() {
	
	int ret, wai;
	bool led_is_on = true;

	if (!device_is_ready(led)) {
		printk("LED is not ready!");
		return;
	}

	if (!device_is_ready(spi1)) {
		printk("spi1 is not ready!");
		return;
	}

	ret = read_wai();

	if (ret < 0) {
		printk("wai invalid!");
		return;
	} else {
		wai = ret;
		printk("wai is %d", wai);
	}


	ret = gpio_pin_configure(led, LED_PIN, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_HIGH);
	if (ret < 0) {
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
			pwm_pin_set_usec(pwm_dev1, 3, 20000, 2000, 0);
			pwm_pin_set_usec(pwm_dev1, 2, 20000, 2000, 0);
			pwm_pin_set_usec(pwm_dev2, 1, 20000, 2000, 0);
		} else {
			ret = pwm_pin_set_usec(pwm_dev1, 4, 20000, 1000, 0);
			pwm_pin_set_usec(pwm_dev1, 3, 20000, 1000, 0);
			pwm_pin_set_usec(pwm_dev1, 2, 20000, 1000, 0);
			pwm_pin_set_usec(pwm_dev2, 1, 20000, 1000, 0);
		}
		printk("%d\n", ret);

		k_sleep(K_MSEC(1500));
	}
}
