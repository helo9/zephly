/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>

#ifndef ZEPHLY_DRIVERS_MPU6000_H_
#define ZEPHLY_DRIVERS_MPU6000_H_

#define ICM20602_REG_SMPLRT_DIV 0x19U

#define ICM20602_REG_CONFIG	0x1AU
#define ICM20602_REG_GYRO_CONFIG	0x1BU
#define ICM20602_REG_ACCEL_CONFIG	0x1CU
#define ICM20602_REG_ACCEL_CONFIG2	0x1DU

#define ICM20602_REG_FIFO_EN	0x023U

#define ICM20602_REG_INT_PIN_CFG 0x37U
#define ICM20602_REG_INT_ENABLE 0x38U

#define ICM20602_REG_ACCEL_XOUT_H	0x3BU
#define ICM20602_REG_ACCEL_XOUT_L	0x3CU
#define ICM20602_REG_ACCEL_YOUT_H	0x3DU
#define ICM20602_REG_ACCEL_YOUT_L	0x3EU
#define ICM20602_REG_ACCEL_ZOUT_H	0x3FU
#define ICM20602_REG_ACCEL_ZOUT_L	0x40U

#define ICM20602_REG_TEMP_OUT_H	0x41U
#define ICM20602_REG_TEMP_OUT_L	0x42U

#define ICM20602_REG_GYRO_XOUT_H	0x43U
#define ICM20602_REG_GYRO_XOUT_L	0x44U
#define ICM20602_REG_GYRO_YOUT_H	0x45U
#define ICM20602_REG_GYRO_YOUT_L	0x46U
#define ICM20602_REG_GYRO_ZOUT_H	0x47U
#define ICM20602_REG_GYRO_ZOUT_L	0x48U

#define ICM20602_REG_DATA_START ICM20602_REG_ACCEL_XOUT_H
#define ICM20602_REG_DATA_LEN (ICM20602_REG_GYRO_ZOUT_L - ICM20602_REG_ACCEL_XOUT_H + 1U)

#define ICM20602_REG_POWER_MANAGEMENT_1 0x6BU
#define ICM20602_REG_POWER_MANAGEMENT_2 0x6CU

#define ICM20602_REG_I2C_INTERFACE 0x70U

#define ICM20602_REG_WHO_AM_I	0x75U

#define ICM20602_SPI_WRITE_FLAG	0x00U
#define ICM20602_SPI_READ_FLAG	BIT(7)

#define ICM20602_WHO_AM_I	0x12U

#ifdef CONFIG_ICM20602_TRIGGER
int icm20602_trigger_set(const struct device *dev, const struct sensor_trigger *trig, sensor_trigger_handler_t handler);
int icm20602_trigger_init(const struct device *dev);
#endif

struct icm20602_config {
    struct spi_dt_spec spi;
#ifdef CONFIG_ICM20602_TRIGGER
	const struct gpio_dt_spec gpio_data_rdy;
#endif
};

struct icm20602_data {
    int16_t accel_measurement[3];
	uint16_t accel_sensitivity_shift;

	int16_t temp;

	int16_t gyro_measurement[3];
	uint16_t gyro_sensitivity_x10;

#ifdef CONFIG_ICM20602_TRIGGER
	struct gpio_callback gpio_cb;
	sensor_trigger_handler_t sensor_cb;
#endif
};

#endif /* ZEPHLY_DRIVERS_MPU6000_H_ */
