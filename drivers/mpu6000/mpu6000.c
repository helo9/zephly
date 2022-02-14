/*
* Copyright (c) 2022 Jonathan Hahn
* SPDX-License-Identifier: Apache-2.0
*/

#include <init.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <sys/byteorder.h>
#include <sys/types.h>
#include "mpu6000.h"

#define DT_DRV_COMPAT invensense_mpu6000

static uint16_t gyro_sensitivity_x10[4] = {1310, 655, 328, 164};

int mpu6000_spi_read(const struct device *dev, uint8_t reg,
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

int mpu6000_spi_read_reg(const struct device *dev, uint8_t reg, uint8_t *val) {
    return mpu6000_spi_read(dev, reg, val, 1);
}

int mpu6000_spi_write(const struct device *dev, uint8_t reg,
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

int mpu6000_spi_write_reg(const struct device *dev, uint8_t reg,
            uint8_t value) {
    return mpu6000_spi_write(dev, reg, &value, 1);
}

static void mpu6000_convert_accel(struct sensor_value *val, int16_t raw_value, uint16_t sensitivity_shift) {
    int64_t conv_val = ((int64_t)raw_value * SENSOR_G) >> sensitivity_shift;
    
    val->val1 = conv_val / 1000000LL;
    val->val2 = conv_val % 1000000LL;
}

static void mpu6000_convert_gyro(struct sensor_value *val, int16_t raw_value, uint16_t sensitivity_x10) {
    int64_t conv_val = ((int64_t)raw_value * SENSOR_PI * 10) / (sensitivity_x10 * 180);

    val->val1 = conv_val / 1000000LL;
    val->val2 = conv_val % 1000000LL;
}

static void mpu6000_convert_temp(struct sensor_value *val, int16_t raw_value) {
    int64_t conv_val = ((int64_t)raw_value * 1000000LL) / 340 + 36530000LL;

    val->val1 = conv_val / 1000000LL;
    val->val2 = conv_val % 1000000LL;
}

static int mpu6000_init(const struct device *dev) {
    struct mpu6000_data *data = dev->data;
    const struct mpu6000_config *config = dev->config;

    if (!spi_is_ready(&config->spi)) {
        printk("SPI not ready\n");
        return -ENODEV;
    }

    /* random wakeup time */
    k_sleep(K_MSEC(500));

    /* wake up device */
    int ret = mpu6000_spi_write_reg(dev, MPU6000_REG_PWR_MGMT1, 0);
    if ( ret != 0 ) {
        return ret;
    }

    /* check who am i register for correct value */
    uint8_t wai = 0;
    ret = mpu6000_spi_read(dev, MPU6000_REG_WHO_AM_I, &wai, 1);
    if ( ret != 0 ) {
        printk("WAI request failed\n");
        return ret;
    }

    if ( wai != MPU6000_WHO_AM_I ) {
        printk("WAI is wrong %d\n", wai);
        return -ENODEV;
    }

    /* Set gyro range */
    data->gyro_sensitivity_x10 = gyro_sensitivity_x10[CONFIG_MPU6000_FS_SEL];
    ret = mpu6000_spi_write_reg(dev, MPU6050_REG_GYRO_CFG, CONFIG_MPU6000_FS_SEL << 3);
    if ( ret != 0 ) {
        printk("Failed setting GYRO CFG\n");
        return ret;
    }

    /* Set accel range */
    data->accel_sensitivity_shift = 14 - CONFIG_MPU6000_AFS_SEL;
    ret = mpu6000_spi_write_reg(dev, MPU6050_REG_ACCEL_CFG, CONFIG_MPU6000_AFS_SEL << 3);
    if ( ret != 0 ) {
        printk("Failed setting GYRO CFG\n");
        return ret;
    }

    /* Configure low pass filter */
    ret = mpu6000_spi_write_reg(dev, MPU6000_REG_DLPF_CFG, CONFIG_MPU6000_DLPF_CONFIG);
    if (ret != 0) {
        printk("Failed configuring low pass filter\n");
        return ret;
    }

    return 0;
}

static int mpu6000_set(const struct device *dev,
            enum sensor_channel chan,
            enum sensor_attribute attr,
            const struct sensor_value *val) {
    return -ENOTSUP;
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
            enum sensor_channel chan, struct sensor_value *val) {
    struct mpu6000_data *data = dev->data;
    
    switch (chan) {
    case SENSOR_CHAN_ACCEL_XYZ:
        mpu6000_convert_accel(&val[0], data->accel_x, data->accel_sensitivity_shift);
        mpu6000_convert_accel(&val[1], data->accel_y, data->accel_sensitivity_shift);
        mpu6000_convert_accel(&val[2], data->accel_z, data->accel_sensitivity_shift);
        break;
    case SENSOR_CHAN_ACCEL_X:
        mpu6000_convert_accel(&val[0], data->accel_x, data->accel_sensitivity_shift);
        break;
    case SENSOR_CHAN_ACCEL_Y:
        mpu6000_convert_accel(&val[0], data->accel_y, data->accel_sensitivity_shift);
        break;
    case SENSOR_CHAN_ACCEL_Z:
        mpu6000_convert_accel(&val[0], data->accel_z, data->accel_sensitivity_shift);
        break;
    case SENSOR_CHAN_GYRO_XYZ:
        mpu6000_convert_gyro(&val[0], data->gyro_x, data->gyro_sensitivity_x10);
        mpu6000_convert_gyro(&val[1], data->gyro_y, data->gyro_sensitivity_x10);
        mpu6000_convert_gyro(&val[2], data->gyro_z, data->gyro_sensitivity_x10);
        break;
    case SENSOR_CHAN_GYRO_X:
        mpu6000_convert_gyro(&val[0], data->gyro_x, data->gyro_sensitivity_x10);
        break;
    case SENSOR_CHAN_GYRO_Y:
        mpu6000_convert_gyro(&val[0], data->gyro_y, data->gyro_sensitivity_x10);
        break;
    case SENSOR_CHAN_GYRO_Z:
        mpu6000_convert_gyro(&val[0], data->gyro_z, data->gyro_sensitivity_x10);
        break;
    case SENSOR_CHAN_DIE_TEMP:
        mpu6000_convert_temp(&val[0], data->temp);
        break;
    default:
        /* an invalid channel was selcted */
        return -EINVAL;
    }

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
        .spi = SPI_DT_SPEC_INST_GET(i, SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8), 2), \
    };    \
    DEVICE_DT_INST_DEFINE(i, &mpu6000_init, NULL,    \
    	    &mpu6000_data_##i, &mpu6000_config_##i,     \
		    POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,   \
		    &mpu6000_api);

DT_INST_FOREACH_STATUS_OKAY(CREATE_MPU6000_INIT)
