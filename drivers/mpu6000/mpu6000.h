/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/spi.h>

#ifndef FFC_DRIVERS_MPU6000_H_
#define FFC_DRIVERS_MPU6000_H_

#define MPU6000_SPI_READ_FLAG   BIT(7)

#define MPU6000_REG_DLPF_CFG	0x1AU
#define MPU6050_REG_GYRO_CFG	0x1BU
#define MPU6050_REG_ACCEL_CFG   0x1CU

#define MPU6000_REG_DATA_START	0x3BU

#define MPU6000_REG_PWR_MGMT1	0x6BU

#define MPU6000_REG_WHO_AM_I    0x75U
#define MPU6000_WHO_AM_I        0x68U

struct mpu6000_config {
    struct spi_dt_spec spi;
};

struct mpu6000_data {
    int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
	uint16_t accel_sensitivity_shift;

	int16_t temp;

	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
	uint16_t gyro_sensitivity_x10;
};

int mpu6000_spi_read(const struct device *dev, uint8_t reg,
				 uint8_t *data, uint16_t len);
int mpu6000_spi_read_reg(const struct device *dev, uint8_t reg, uint8_t *val);
int mpu6000_spi_write(const struct device *dev, uint8_t reg,
                    uint8_t *data, uint16_t len);
int mpu6000_spi_write_reg(const struct device *dev, uint8_t reg,
            uint8_t value);

#endif /* FFC_DRIVERS_MPU6000_H_ */
