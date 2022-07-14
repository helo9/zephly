/*
* Copyright (c) 2022 Jonathan Hahn
* SPDX-License-Identifier: Apache-2.0
*/

#include "drivers/gpio.h"
#include "icm20602.h"
#include <drivers/sensor.h>
#include <errno.h>

static void icm20602_gpio_interrupt_callback(const struct device *dev,
				      struct gpio_callback *cb, uint32_t pins) {

    static const struct sensor_trigger trigger = {
        .type = SENSOR_TRIG_DATA_READY,
        .chan = SENSOR_CHAN_GYRO_XYZ,
    };

    struct icm20602_data *data = CONTAINER_OF(cb, struct icm20602_data, gpio_cb);
    const struct device *sensor_dev = CONTAINER_OF(data, struct device, data);

    if (data->sensor_cb == NULL) {
        return;
    }

    data->sensor_cb(sensor_dev, &trigger);
}

int icm20602_trigger_init(const struct device *dev) {
    const struct icm20602_config *cfg = dev->config;
    const struct gpio_dt_spec *gpio_spec = &cfg->gpio_data_rdy;
    struct icm20602_data *data = dev->data;

    if (gpio_spec->port == NULL || !z_device_is_ready(gpio_spec->port)) {
        return -ENODEV;
    }

    gpio_init_callback(&data->gpio_cb, icm20602_gpio_interrupt_callback, BIT(gpio_spec->pin));
    
    int ret = gpio_add_callback(gpio_spec->port, &data->gpio_cb);
    if (ret != 0) {
        return ret;
    }

    ret = gpio_pin_configure_dt(gpio_spec, GPIO_INPUT);
    if (ret != 0) {
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(gpio_spec, GPIO_INT_EDGE_RISING);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

int icm20602_trigger_set(const struct device *dev, const struct sensor_trigger *trig, sensor_trigger_handler_t handler) {
    const struct icm20602_config *cfg = dev->config;
    struct icm20602_data *data = dev->data;
    
    if (trig->type != SENSOR_TRIG_DATA_READY) {
        return -ENOTSUP;
    }

    if (trig->chan != SENSOR_CHAN_GYRO_XYZ && trig->chan != SENSOR_CHAN_ACCEL_XYZ) {
        return -ENOTSUP;
    }

    if (cfg->gpio_data_rdy.port == NULL) {
        return -ENOTSUP;
    }

    if (handler == NULL) {
        return -EINVAL;
    }

    int ret = gpio_remove_callback(cfg->gpio_data_rdy.port, &data->gpio_cb);
    if (ret != 0) {
        return ret;
    }

    data->sensor_cb = handler;

    ret = gpio_add_callback(cfg->gpio_data_rdy.port, &data->gpio_cb);

    return ret;
}
