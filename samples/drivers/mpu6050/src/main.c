/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <drivers/sensor.h>
#include <drivers/i2c.h>
#include <shell/shell.h>

#include <string.h>
#include <stdio.h>


/* The devicetree node identifier for the "led0" alias. */
#define LED_NODE DT_ALIAS(userled0)
#define LED_GPIO DT_PHANDLE(LED_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED_NODE, gpios)
const struct device *led = DEVICE_DT_GET(LED_GPIO);
const struct device *mpu6050 = DEVICE_DT_GET(DT_NODELABEL(imu));
const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c2));

bool running = false;

static int cmd_get_reg(const struct shell *sh, size_t argc, char **argv)
{
    uint8_t reg = (uint8_t)strtol(argv[1], NULL, 16);

    uint8_t val;
    int ret = i2c_reg_read_byte(i2c_dev, (uint16_t) 0x68 , reg, &val);

    if ( ret != 0 ) {
        printk("Couldn't read register (%d)\n", ret);
        return ret;
    } 

    printk("Reg 0x%x: 0x%x\n", reg, val);
    
    return 0;
}

static int cmd_set_reg(const struct shell *sh, size_t argc, char **argv)
{
    uint8_t reg = (uint8_t)strtol(argv[1], NULL, 16);
    uint8_t val = (uint8_t)strtol(argv[2], NULL, 16);

    int ret = i2c_reg_write_byte(i2c_dev, (uint16_t) 0x68, reg, val);

    if ( ret != 0 ) {
        printk("Error writing register 0x%x\n", reg);
        return reg;
    }

    printk("Reg 0x%x was set to 0x%x\n", reg, val);
	
    return 0;
}

static int cmd_activate(const struct shell *sh, size_t argc, char **argv)
{
    running = true;
    return 0;
}

static int cmd_deactivate(const struct shell *sh, size_t argc, char **argv)
{
    running = false;
    return 0;
}

SHELL_CMD_ARG_REGISTER(get_reg, NULL, "Read MPU6050 register", cmd_get_reg, 2, 0);
SHELL_CMD_ARG_REGISTER(set_reg, NULL, "Set MPU6050 register", cmd_set_reg, 3, 0);
SHELL_CMD_REGISTER(activate, NULL, "Print MPU6000 data", cmd_activate);
SHELL_CMD_REGISTER(deactivate, NULL, "Stop printing MPU6000 data", cmd_deactivate);

void main(void)
{
	bool led_is_on = true;
	int ret;

	if (!device_is_ready(led)) {
        printk("LED is not ready!");
        return;
    }

	ret = gpio_pin_configure(led, PIN, GPIO_OUTPUT_HIGH | GPIO_ACTIVE_HIGH);
	if (ret < 0) {
		return;
	}

    if (!device_is_ready(mpu6050)) {
        printk("MPU6050 is not ready!");
        return;
    }

    // This needs to be done to wake up the MPU6050. Temporary solution
    // till the zephyr upstream driver is fixed.
    ret = i2c_reg_write_byte(i2c_dev, (uint16_t) 0x68, 0x6B, 0x00);

    if ( ret != 0 ) {
        printk("Error wakeing up the MPU6050");
    }

	while (1) {

		gpio_pin_set(led, PIN, (int)led_is_on);
		led_is_on = !led_is_on;

        if (running) {
            ret = sensor_sample_fetch(mpu6050);

            struct sensor_value gyro[3];
            struct sensor_value accel[3];
            struct sensor_value temp;

            sensor_channel_get(mpu6050, SENSOR_CHAN_GYRO_XYZ, gyro);
            sensor_channel_get(mpu6050, SENSOR_CHAN_ACCEL_XYZ, accel);
            sensor_channel_get(mpu6050, SENSOR_CHAN_DIE_TEMP, &temp);

            if (ret != 0) {
                printk("error occured while collecting data (%d)\n", ret);

            } else {
                printf("%f\t%f\t%f\t-\t%f\t%f\t%f\t-\t%f\n", 
                    sensor_value_to_double(&gyro[0]),sensor_value_to_double(&gyro[1]),
                    sensor_value_to_double(&gyro[2]),sensor_value_to_double(&accel[0]),
                    sensor_value_to_double(&accel[1]),sensor_value_to_double(&accel[2]),
                    sensor_value_to_double(&temp));
            }
        }

		k_sleep(K_MSEC(500));
	}
}
