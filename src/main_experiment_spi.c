#include <zephyr.h>
#include <zephyr.h>
#include "irq.h"
#include "kernel.h"
#include "sys/util.h"
#include <device.h>
#include <stdio.h>
#include <devicetree.h>
#include <drivers/spi.h>

#define WHO_AM_I_REG 0x75

#define LED_NODE DT_ALIAS(userled0)
#define LED_GPIO DT_PHANDLE(LED_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED_NODE, gpios)

#define SPI1 DT_NODELABEL(spi1)
#define SPI1_CS DT_NODELABEL(gpioa)
#define MPU6000 DT_NODELABEL(imu)
#define SPI1_CS_PIN 4

const struct device *led = DEVICE_DT_GET(LED_GPIO);
const struct device *spi1 = DEVICE_DT_GET(DT_BUS(MPU6000));

struct spi_config spi_cfg = SPI_CONFIG_DT(MPU6000, SPI_OP_MODE_MASTER | SPI_WORD_SET(8), 0);

int request_wai(const struct device* spi1);

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

    if (!device_is_ready(spi1)) {
        printk("SPI is not ready!");
        return;
    }

    while (true) {
        printk("Noch da :)\n");

        ret = gpio_pin_set(led, PIN, (int)led_is_on );
        led_is_on = !led_is_on;

        ret = request_wai(spi1);

        if (ret != 0) {
            printk("Probleme probleme probleme.. (%d)\n", ret);
            ret = 0;
        }
        
        k_sleep(K_MSEC(1000));
    }
}

int request_wai(const struct device* spi1) {

    uint8_t buffer_tx[] = {0xF5, 0x00};
    uint8_t buffer_rx[2] = {};

    struct spi_buf tx_bufs;
    tx_bufs.buf = buffer_tx;
    tx_bufs.len = 2;
    const struct spi_buf_set tx = {
		.buffers = &tx_bufs,
		.count = 1
	};

    struct spi_buf rx_bufs;
    rx_bufs.buf = buffer_rx;
    rx_bufs.len = 2;
    const struct spi_buf_set rx = {
        .buffers = &rx_bufs,
        .count = 1
    };

    int ret = spi_transceive(spi1, &spi_cfg, &tx, &rx);

    if (ret == 0) {
        printk("WAI: %d\n", buffer_rx[1]);
    }

    return ret;
}

