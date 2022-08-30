/*
* Copyright (c) 2022 Jonathan Hahn
*
* SPDX-License-Identifier: Apache-2.0
*/

#include "devicetree.h"
#include "sys/util.h"
#include <zephyr.h>
#include <device.h>

#include <custom_drivers/rc.h>
#include <sbus_parser.h>

#define DT_DRV_COMPAT futaba_sbusreceiver

struct sbus_data {
    struct SBusBuffer buffers[2];
    uint8_t active_buffer_id;

    struct SBusData received;

    struct k_work parse_work;
};

struct sbus_config {
    const struct device *uart;
};

static inline void sbus_switch_buffer(struct sbus_data *data){
    data->active_buffer_id = !data->active_buffer_id;
    data->buffers[data->active_buffer_id].pointer = 0;
}

static inline void sbus_trigger_parsing(const struct device *dev) {
    struct sbus_data *data = dev->data;
    k_work_submit(&data->parse_work);
}

static void sbus_handle_rx(const struct device *dev) {
    struct sbus_data *data = dev->data;
    const struct sbus_config *config = dev->config;

    struct SBusBuffer *active_buffer = &data->buffers[data->active_buffer_id];

    uint8_t tmp;
    while (uart_fifo_read(config->uart, &tmp, 1) == 1) {
        
        sbus_add_to_buffer(active_buffer, tmp);

        if ( sbus_is_buffer_full(active_buffer)) {
            sbus_switch_buffer(data);
            sbus_trigger_parsing(dev);
        }
    }
}

static void sbus_uart_cb(const struct device *uart, void *dev_ptr) {
    uart_irq_update(uart);

    if (uart_irq_rx_ready(uart)) {
        sbus_handle_rx((const struct device*)dev_ptr);
    }
}

static inline void sbus_init_uart_callbacks(const struct device *dev) {
    const struct sbus_config *config = dev->config;

    uart_irq_rx_disable(config->uart);
	uart_irq_tx_disable(config->uart);
    uart_irq_callback_user_data_set(config->uart, sbus_uart_cb, (void*)dev);
    uart_irq_rx_enable(config->uart);
}

static void sbus_parse_work_handler(struct k_work *work) {
    struct sbus_data *data = CONTAINER_OF(work, struct sbus_data, parse_work);

    struct SBusBuffer *inactive_buf = &data->buffers[!data->active_buffer_id];

    sbus_parse_buffer(inactive_buf, &data->received);
}

static int sbus_receiver_init(const struct device *dev) {
    struct sbus_data *data = dev->data;
    const struct sbus_config *config = dev->config;

    if (!device_is_ready(config->uart)) {
        printk("UART not ready.");
        return -EIO;
    }

    struct uart_config cfg;
    
    int ret = uart_config_get(config->uart, &cfg);

    if (ret != 0) {
        return ret;
    }

    cfg.stop_bits = UART_CFG_STOP_BITS_2;
    cfg.data_bits = UART_CFG_DATA_BITS_8;
    cfg.parity = UART_CFG_PARITY_EVEN;
    
    ret = uart_configure(config->uart, &cfg);

    if (ret != 0) {
        return ret;
    }

    printk("Initializing SBUS Receiver!\n");

    sbus_init_uart_callbacks(dev);

    k_work_init(&data->parse_work, sbus_parse_work_handler);
    
    return 0;
}

static void sbus_update(const struct device *dev, struct Command *rc_val) {
    struct sbus_data *data = dev->data;
    const struct SBusData *recv_data = &data->received;
    rc_val->armed = true;
    rc_val->pitch = recv_data->servo_channels[1] / 991.5f - 1.0f;
    rc_val->roll = recv_data->servo_channels[0] / 991.5f - 1.0f;
    rc_val->thrust = recv_data->servo_channels[2] / 991.5f - 1.0f;
    rc_val->yaw = recv_data->servo_channels[3] / 991.5f - 1.0f;
}

struct rc_api sbus_api = {
    .update = sbus_update
};

#define SBUS_RC_INIT(i) \
    struct sbus_data sbus_data_##i = {}; \
    struct sbus_config sbus_config_##i = { \
        .uart = DEVICE_DT_GET(DT_INST_BUS(i)) \
    }; \
    DEVICE_DT_INST_DEFINE(i, &sbus_receiver_init, NULL, \
            &sbus_data_##i, &sbus_config_##i, \
            POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, \
            &sbus_api);

DT_INST_FOREACH_STATUS_OKAY(SBUS_RC_INIT)
