#include <zephyr.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <shell/shell.h>
#include <mpu6000.h>
#include <stdio.h>
#include <stdlib.h>

#define LED_NODE DT_ALIAS(userled0)
#define LED_GPIO DT_PHANDLE(LED_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED_NODE, gpios)

#define MPU6000 DT_NODELABEL(imu)

const struct device *led = DEVICE_DT_GET(LED_GPIO);
const struct device *imu = DEVICE_DT_GET(MPU6000);

bool running = false;

static int cmd_get_reg(const struct shell *sh, size_t argc, char **argv)
{
    uint8_t reg = (uint8_t)strtol(argv[1], NULL, 16);

    uint8_t val;
    int ret = mpu6000_spi_read_reg(imu, reg, &val);

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

    int ret = mpu6000_spi_write_reg(imu, reg, val);

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

SHELL_CMD_ARG_REGISTER(get_reg, NULL, "Read MPU6000 register", cmd_get_reg, 2, 0);
SHELL_CMD_ARG_REGISTER(set_reg, NULL, "Set MPU6000 register", cmd_set_reg, 3, 0);
SHELL_CMD_REGISTER(activate, NULL, "Set MPU6000 register", cmd_activate);
SHELL_CMD_REGISTER(deactivate, NULL, "Set MPU6000 register", cmd_deactivate);

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

    printk("Succesfully started :)\n");

    while (true) {
        
        ret = gpio_pin_set(led, PIN, (int)led_is_on );
        led_is_on = !led_is_on;

        if (running) {
            ret = sensor_sample_fetch(imu);

            struct sensor_value gyro[3];
            struct sensor_value accel[3];
            struct sensor_value temp;

            sensor_channel_get(imu, SENSOR_CHAN_GYRO_XYZ, gyro);
            sensor_channel_get(imu, SENSOR_CHAN_ACCEL_XYZ, accel);
            sensor_channel_get(imu, SENSOR_CHAN_DIE_TEMP, &temp);

            if (ret != 0) {
                printk("error occured while collecting data (%d)\n", ret);
                ret = 0;
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
