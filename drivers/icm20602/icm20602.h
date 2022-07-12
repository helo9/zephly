/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/spi.h>

#ifndef ZEPHLY_DRIVERS_MPU6000_H_
#define ZEPHLY_DRIVERS_MPU6000_H_

#define ICM20602_REG_CONFIG	0x1AU
#define ICM20602_REG_GYRO_CONFIG	0x1BU
#define ICM20602_REG_ACCEL_CONFIG	0x1CU
#define ICM20602_REG_ACCEL_CONFIG2	0x1DU

#define ICM20602_REG_FIFO_EN	0x023U

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

struct icm20602_config {
    struct spi_dt_spec spi;
};

struct icm20602_data {
    int16_t accel_measurement[3];
	uint16_t accel_sensitivity_shift;

	int16_t temp;

	int16_t gyro_measurement[3];
	int64_t gyro_offsets[3];
	uint16_t gyro_sensitivity_x10;

	struct k_mutex *gyro_offset_mutex;
};

#endif /* ZEPHLY_DRIVERS_MPU6000_H_ */
