/*
* Copyright (c) 2022 Jonathan Hahn
* SPDX-License-Identifier: Apache-2.0
*/

#include <init.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <sys/byteorder.h>
#include <sys/types.h>
#include "icm20602.h"

#define DT_DRV_COMPAT invensense_icm20602

const static uint16_t gyro_sensitivities_x10[4] = {1310, 655, 328, 164};

int icm20602_spi_read(const struct device *dev, uint8_t reg,
				 uint8_t *data, uint16_t len)
{
	const struct icm20602_config *config = dev->config;
	uint8_t buffer_tx[2] = { reg | ICM20602_SPI_READ_FLAG, 0 };
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

int icm20602_spi_read_reg(const struct device *dev, uint8_t reg, uint8_t *val) {
    return icm20602_spi_read(dev, reg, val, 1);
}

int icm20602_spi_write(const struct device *dev, uint8_t reg,
                    uint8_t *data, uint16_t len) {
	const struct icm20602_config *config = dev->config;
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

int icm20602_spi_write_reg(const struct device *dev, uint8_t reg,
            uint8_t value) {
    return icm20602_spi_write(dev, reg, &value, 1);
}

static void icm20602_convert_accel(struct sensor_value *val, int16_t raw_value, uint16_t sensitivity_shift) {
    int64_t conv_val = ((int64_t)raw_value * SENSOR_G) >> sensitivity_shift;
    
    val->val1 = conv_val / 1000000LL;
    val->val2 = conv_val % 1000000LL;
}

static void icm20602_convert_gyro(struct sensor_value *val, int16_t raw_value, uint16_t sensitivity_x10, int64_t offset) {
    int64_t conv_val = ((int64_t)raw_value * SENSOR_PI * 10) / (sensitivity_x10 * 180) + offset;

    val->val1 = conv_val / 1000000LL;
    val->val2 = conv_val % 1000000LL;
}

static void icm20602_convert_temp(struct sensor_value *val, int16_t raw_value) {
    int64_t conv_val = ((int64_t)raw_value * 1000000LL) / 340 + 36530000LL;

    val->val1 = conv_val / 1000000LL;
    val->val2 = conv_val % 1000000LL;
}

static int icm20602_init(const struct device *dev) {
    struct icm20602_data *data = dev->data;
    const struct icm20602_config *config = dev->config;

    if (!spi_is_ready(&config->spi)) {
        printk("SPI not ready\n");
        return -ENODEV;
    }

    /* random wakeup time */
    k_sleep(K_MSEC(500));


    /* check who am i register for correct value */
    uint8_t wai = 0;
    int ret = icm20602_spi_read(dev, ICM20602_REG_WHO_AM_I, &wai, 1);
    if ( ret != 0 ) {
        printk("WAI request failed\n");
        return ret;
    }

    if ( wai != ICM20602_WHO_AM_I ) {
        printk("WAI is wrong (%d)\n", wai);
        return -ENODEV;
    }

    /* Set gyro range */
    data->gyro_sensitivity_x10 = gyro_sensitivities_x10[CONFIG_ICM20602_FS_SEL];
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_GYRO_CONFIG, CONFIG_ICM20602_FS_SEL << 3);
    if ( ret != 0 ) {
        printk("Failed setting GYRO CFG\n");
        return ret;
    }

    /* Set accel range */
    data->accel_sensitivity_shift = 14 - CONFIG_ICM20602_AFS_SEL;
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_ACCEL_CONFIG, CONFIG_ICM20602_AFS_SEL << 3);
    if ( ret != 0 ) {
        printk("Failed setting GYRO CFG\n");
        return ret;
    }

    /* Configure low pass filter gyro and temperature */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_CONFIG, CONFIG_ICM20602_DLPF_GYRO_TEMP_CONFIG);
    if (ret != 0) {
        printk("Failed configuring low pass filter for gyro and temperature\n");
        return ret;
    }

    /* Configure low pass filter accelerometer */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_ACCEL_CONFIG2, CONFIG_ICM20602_DLPF_ACCEL_CONFIG);
    if (ret != 0) {
        printk("Failed configuring low pass filter for gyro and temperature\n");
        return ret;
    }

    return 0;
}

#if 0
static int icm20602_set(const struct device *dev,
            enum sensor_channel chan,
            enum sensor_attribute attr,
            const struct sensor_value *val) {

    struct icm20602_data *data = dev->data;
    int i, j;

    if (attr != SENSOR_ATTR_OFFSET) {
        return -ENOTSUP;
    }

    switch (chan) {
    case SENSOR_CHAN_GYRO_XYZ:
        i = 0; j = 2;
        break;
    case SENSOR_CHAN_GYRO_X:
        i = 0; j = 0;
        break;
    case SENSOR_CHAN_GYRO_Y:
        i = 1; j = 1;
        break;
    case SENSOR_CHAN_GYRO_Z:
        i = 2; j = 2;
        break;
    default:
        return -ENOTSUP;
    }

    if (k_mutex_lock(data->gyro_offset_mutex, K_MSEC(100)) == 0) {
        for (; i<=j; i++) {
            data->gyro_offsets[i] = val->val1 * 1000000LL + val->val2;
        }
    } else {
        printk("Cannot lock icm20602 configuration.\n");
    }

    k_mutex_unlock(data->gyro_offset_mutex);
    
    return 0;   
}
#endif

static int icm20602_fetch(const struct device *dev,
                        enum sensor_channel chan) {
    struct icm20602_data *data = dev->data;

    if ((chan != SENSOR_CHAN_ALL)) {
        return -ENOTSUP;
    }
    int16_t buf[7];

    int ret = icm20602_spi_read(dev, ICM20602_REG_DATA_START, (uint8_t*)buf,  ICM20602_REG_DATA_LEN);
    if ( ret != 0 ) {
        return ret;
    }

    for (int i=0; i<3; i++) {
        data->accel_measurement[i] = sys_be16_to_cpu(buf[i]);
    }

	data->temp = sys_be16_to_cpu(buf[3]);

    for (int i=0; i<3; i++) {
        data->gyro_measurement[i] = sys_be16_to_cpu(buf[4+i]);
    }

    return 0;
}

static int64_t icm20602_get_gyro_offset(const struct device *dev, uint8_t gyro_chan) {
    return 0;
}

static int icm20602_get(const struct device *dev,
            enum sensor_channel chan, struct sensor_value *val) {
    struct icm20602_data *data = dev->data;
    
    switch (chan) {
    case SENSOR_CHAN_ACCEL_XYZ:
        icm20602_convert_accel(&val[0], data->accel_measurement[0], data->accel_sensitivity_shift);
        icm20602_convert_accel(&val[1], data->accel_measurement[1], data->accel_sensitivity_shift);
        icm20602_convert_accel(&val[2], data->accel_measurement[2], data->accel_sensitivity_shift);
        break;
    case SENSOR_CHAN_ACCEL_X:
        icm20602_convert_accel(&val[0], data->accel_measurement[0], data->accel_sensitivity_shift);
        break;
    case SENSOR_CHAN_ACCEL_Y:
        icm20602_convert_accel(&val[0], data->accel_measurement[1], data->accel_sensitivity_shift);
        break;
    case SENSOR_CHAN_ACCEL_Z:
        icm20602_convert_accel(&val[0], data->accel_measurement[2], data->accel_sensitivity_shift);
        break;
    case SENSOR_CHAN_GYRO_XYZ:
        icm20602_convert_gyro(&val[0], data->gyro_measurement[0], data->gyro_sensitivity_x10, icm20602_get_gyro_offset(dev, 0));
        icm20602_convert_gyro(&val[1], data->gyro_measurement[1], data->gyro_sensitivity_x10, icm20602_get_gyro_offset(dev, 1));
        icm20602_convert_gyro(&val[2], data->gyro_measurement[2], data->gyro_sensitivity_x10, icm20602_get_gyro_offset(dev, 2));
        break;
    case SENSOR_CHAN_GYRO_X:
        icm20602_convert_gyro(&val[0], data->gyro_measurement[0], data->gyro_sensitivity_x10, icm20602_get_gyro_offset(dev, 0));
        break;
    case SENSOR_CHAN_GYRO_Y:
        icm20602_convert_gyro(&val[0], data->gyro_measurement[1], data->gyro_sensitivity_x10, icm20602_get_gyro_offset(dev, 1));
        break;
    case SENSOR_CHAN_GYRO_Z:
        icm20602_convert_gyro(&val[0], data->gyro_measurement[2], data->gyro_sensitivity_x10, icm20602_get_gyro_offset(dev, 2));
        break;
    case SENSOR_CHAN_DIE_TEMP:
        icm20602_convert_temp(&val[0], data->temp);
        break;
    default:
        /* an invalid channel was selcted */
        return -EINVAL;
    }

    return 0;
}

static const struct sensor_driver_api icm20602_api = {
	.attr_set = NULL,
	.sample_fetch = icm20602_fetch,
	.channel_get = icm20602_get,
};

#define CREATE_ICM20602_INIT(i)                        \
    K_MUTEX_DEFINE(icm20602_mutex_##i);   \
    static struct icm20602_data icm20602_data_##i = {       \
        .gyro_offset_mutex = &icm20602_mutex_##i, \
    };  \
    static struct icm20602_config icm20602_config_##i = { \
        .spi = SPI_DT_SPEC_INST_GET(i, SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8), 2), \
    };    \
    DEVICE_DT_INST_DEFINE(i, &icm20602_init, NULL,    \
    	    &icm20602_data_##i, &icm20602_config_##i,     \
		    POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,   \
		    &icm20602_api);

DT_INST_FOREACH_STATUS_OKAY(CREATE_ICM20602_INIT)
