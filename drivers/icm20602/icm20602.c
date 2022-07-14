/*
* Copyright (c) 2022 Jonathan Hahn
* SPDX-License-Identifier: Apache-2.0
*/

#define DT_DRV_COMPAT invensense_icm20602

#include <devicetree.h>
#include <sys/byteorder.h>
#include <sys/types.h>
#include "icm20602.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(ICM20602, CONFIG_SENSOR_LOG_LEVEL);

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

static inline bool icm20602_spi_is_ready(const struct device *dev) {
    const struct icm20602_config *config = dev->config;

    return spi_is_ready(&config->spi);
}

static int icm20602_init(const struct device *dev) {
    struct icm20602_data *data = dev->data;
    
    if (!icm20602_spi_is_ready(dev)) {
        LOG_ERR("SPI not ready\n");
        return -ENODEV;
    }

    /* check who am i register for correct value */
    uint8_t wai = 0;
    int ret = icm20602_spi_read(dev, ICM20602_REG_WHO_AM_I, &wai, 1);
    if ( ret != 0 ) {
        LOG_ERR("Requesting Who Am I failed\n");
        return ret;
    }

    if ( wai != ICM20602_WHO_AM_I ) {
        LOG_ERR("Who I Am ID has wrong value (%d)\n", wai);
        return -ENODEV;
    }

    /* reset sensor */
    icm20602_spi_write_reg(dev, ICM20602_REG_POWER_MANAGEMENT_1, BIT(7));

    /* random wakeup time */
    k_sleep(K_MSEC(500));

    /* Disable FIFO mode */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_FIFO_EN, 0x00);
    if ( ret != 0 ) {
        LOG_ERR("Failed to disable FIFO mode\n");
        return ret;
    }

    /* Tinker around with first PWR_MGMT register */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_POWER_MANAGEMENT_1, 0x00);
    if ( ret != 0 ) {
        LOG_ERR("Failed to set PWR_MGMT_1\n");
        return ret;
    }

    /* Do something similiar with second PWR_MGMT register */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_POWER_MANAGEMENT_2, 0x00);
    if ( ret != 0 ) {
        LOG_ERR("Failed to set PWR_MGMT_2\n");
        return ret;
    }

    /* Disable I2C Interface */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_I2C_INTERFACE, BIT(6));
    if ( ret != 0 ) {
        LOG_ERR("Failed to disable I2C interface\n");
        return ret;
    }

    /* Set gyro range */
    data->gyro_sensitivity_x10 = gyro_sensitivities_x10[CONFIG_ICM20602_FS_SEL];
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_GYRO_CONFIG, CONFIG_ICM20602_FS_SEL << 3);
    if ( ret != 0 ) {
        LOG_ERR("Failed setting GYRO CFG\n");
        return ret;
    }

    /* Set accel range */
    data->accel_sensitivity_shift = 14 - CONFIG_ICM20602_AFS_SEL;
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_ACCEL_CONFIG, CONFIG_ICM20602_AFS_SEL << 3);
    if ( ret != 0 ) {
        LOG_ERR("Failed setting GYRO CFG\n");
        return ret;
    }

    /* Configure low pass filter gyro and temperature */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_CONFIG, CONFIG_ICM20602_DLPF_GYRO_TEMP_CONFIG);
    if (ret != 0) {
        LOG_ERR("Failed configuring low pass filter for gyro and temperature\n");
        return ret;
    }

    /* Configure low pass filter accelerometer */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_ACCEL_CONFIG2, CONFIG_ICM20602_DLPF_ACCEL_CONFIG);
    if (ret != 0) {
        LOG_ERR("Failed configuring low pass filter for gyro and temperature\n");
        return ret;
    }

    /* Configure sample rate divider */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_SMPLRT_DIV, CONFIG_ICM20602_SAMPLE_RATE_DIVIDER);
    if (ret != 0) {
        LOG_ERR("Failed configuring low pass filter for gyro and temperature\n");
        return ret;
    }

#if CONFIG_ICM20602_TRIGGER
    /* Configure interrupt pins */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_INT_PIN_CFG, 0x10);
    if (ret != 0) {
        LOG_ERR("Failed configuring interrupt pins\n");
        return ret;
    }

    /* Enable interrupts */
    ret = icm20602_spi_write_reg(dev, ICM20602_REG_INT_ENABLE, BIT(0));
    if (ret != 0) {
        LOG_ERR("Failed enabling data ready interrupt\n");
        return ret;
    }

    ret = icm20602_trigger_init(dev);
    if (ret != 0) {
        LOG_ERR("Failed to initialise interrupt configuration\n");
    }
#endif 

    return 0;
}

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
#if CONFIG_ICM20602_TRIGGER
	.trigger_set = icm20602_trigger_set,
#endif
	.sample_fetch = icm20602_fetch,
	.channel_get = icm20602_get,
};

#if CONFIG_ICM20602_TRIGGER

#define GPIO_DT_SPEC_INST_GET_BY_IDX_COND(id, prop, idx) \
	COND_CODE_1(DT_INST_PROP_HAS_IDX(id, prop, idx), \
		    (GPIO_DT_SPEC_INST_GET_BY_IDX(id, prop, idx)), \
		    ({.port = NULL, .pin = 0, .dt_flags = 0}))

#define ICM20602_TRIGGER_CONFIG(inst) \
    .gpio_data_rdy = \
        GPIO_DT_SPEC_INST_GET_BY_IDX_COND(inst, int_gpios, 0),
#else
#define ICM20602_TRIGGER_CONFIG(int)
#endif

#define CREATE_ICM20602_INIT(i)                        \
    static struct icm20602_data icm20602_data_##i = {       \
    };  \
    static struct icm20602_config icm20602_config_##i = { \
        .spi = SPI_DT_SPEC_INST_GET(i, SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8), 2), \
        ICM20602_TRIGGER_CONFIG(i)\
    };    \
    DEVICE_DT_INST_DEFINE(i, &icm20602_init, NULL,    \
    	    &icm20602_data_##i, &icm20602_config_##i,     \
		    POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,   \
		    &icm20602_api);

DT_INST_FOREACH_STATUS_OKAY(CREATE_ICM20602_INIT)
