/*
* Copyright (c) 2022 Jonathan Hahn
* SPDX-License-Identifier: Apache-2.0
*/

#define DT_DRV_COMPAT invensense_mpu6000

#define MPU6000_SPI_READ_FLAG   BIT(7)

#define MPU6000_REG_DATA_START	0x3BU

#define MPU6000_REG_PWR_MGMT1	0x6BU
#define MPU6000_SLEEP_EN		BIT(6)

#define MPU6000_REG_WHO_AM_I    0x75U
#define MPU6000_WHO_AM_I        0x68U



#include <init.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/spi.h>
#include <drivers/sensor.h>
#include <sys/byteorder.h>

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

static int mpu6000_spi_read(const struct device *dev, uint8_t reg,
				 uint8_t *data, uint16_t len)
{
	const struct mpu6000_config *config = dev->config;
	uint8_t buffer_tx[2] = { reg | MPU6000_SPI_READ_FLAG, 0 };
	const struct spi_buf tx_buf = {
		.buf = buffer_tx,
		.len = 2,
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1,
	};
	const struct spi_buf rx_buf[2] = {
		{
			.buf = NULL,
			.len = 1,
		},
		{
			.buf = data,
			.len = len,
		}
	};
	const struct spi_buf_set rx = {
		.buffers = rx_buf,
		.count = 2,
	};

	int ret = spi_transceive_dt(&config->spi, &tx, &rx);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int mpu6000_spi_write(const struct device *dev, uint8_t reg,
                    uint8_t *data, uint16_t len) {
	const struct mpu6000_config *config = dev->config;
	uint8_t buffer_tx = reg;
	const struct spi_buf tx_buf[2] = {
		{
			.buf = &buffer_tx,
			.len = 1,
		},
		{
			.buf = data,
			.len = len,
		}
	};
	const struct spi_buf_set tx = {
		.buffers = tx_buf,
		.count = 2,
	};

	int ret = spi_write_dt(&config->spi, &tx);
	if (ret < 0) {
		return ret;
	}

    return 0;
}

static int mpu6000_init(const struct device *dev) {
    const struct mpu6000_config *config = dev->config;

    if (!spi_is_ready(&config->spi)) {
        printk("SPI not ready\n");
        return -ENODEV;
    }

    uint8_t wai = 0;
    int ret = mpu6000_spi_read(dev, MPU6000_REG_WHO_AM_I, &wai, 1);
    if ( ret != 0 ) {
        printk("WAI request failed\n");
        return ret;
    }

    /* check who am i register for correct value */
    if ( wai != MPU6000_WHO_AM_I ) {
        printk("WAI is wrong %d", wai);
        return -ENODEV;
    }

    /* wake up device */
    uint8_t data = MPU6000_SLEEP_EN;
    ret = mpu6000_spi_write(dev, MPU6000_REG_PWR_MGMT1, &data, 1);
    if ( ret != 0 ) {
        return ret;
    }

    return 0;
}

static int mpu6000_set(const struct device *dev,
            enum sensor_channel chan,
            enum sensor_attribute attr,
            const struct sensor_value *val) {
    return 0;
}

static int mpu6000_fetch(const struct device *dev,
                        enum sensor_channel chan) {
    struct mpu6000_data *data = dev->data;

    if ((chan != SENSOR_CHAN_ALL) && (chan != SENSOR_CHAN_GYRO_XYZ)) {
        return -ENOTSUP;
    }
    int16_t buf[7];

    int ret = mpu6000_spi_read(dev, MPU6000_REG_DATA_START, (uint8_t*)buf,  14);
    if ( ret != 0 ) {
        return ret;
    }

    data->accel_x = sys_be16_to_cpu(buf[0]);
	data->accel_y = sys_be16_to_cpu(buf[1]);
	data->accel_z = sys_be16_to_cpu(buf[2]);
	data->temp = sys_be16_to_cpu(buf[3]);
	data->gyro_x = sys_be16_to_cpu(buf[4]);
	data->gyro_y = sys_be16_to_cpu(buf[5]);
	data->gyro_z = sys_be16_to_cpu(buf[6]);
    
    return 0;
}

static int mpu6000_get(const struct device *dev,
            enum sensor_channel chan,
            struct sensor_value *val) {
    return 0;
}

static const struct sensor_driver_api mpu6000_api = {
	.attr_set = mpu6000_set,
	.sample_fetch = mpu6000_fetch,
	.channel_get = mpu6000_get,
};

#define CREATE_MPU6000_INIT(i)                        \
    static struct mpu6000_data mpu6000_data_##i;        \
    static struct mpu6000_config mpu6000_config_##i = { \
        .spi = SPI_DT_SPEC_INST_GET(i, SPI_OP_MODE_MASTER | SPI_WORD_SET(8), 1), \
    };    \
    DEVICE_DT_INST_DEFINE(i, &mpu6000_init, NULL,    \
    	    &mpu6000_data_##i, &mpu6000_config_##i,     \
		    POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,   \
		    &mpu6000_api);

DT_INST_FOREACH_STATUS_OKAY(CREATE_MPU6000_INIT)
