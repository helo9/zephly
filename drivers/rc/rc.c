/*
* Copyright (c) 2022 Jonathan Hahn
* SPDX-License-Identifier: Apache-2.0
*/

#include <init.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/uart.h>
#include <stdint.h>
#include <string.h>
#include "spm_srxl.h"
#include "custom_drivers/rc.h"

#define DT_DRV_COMPAT spektrum_srxl2receiver

#define SRXL_DEVICE_ID  0x31
#define SRXL_DEVICE_PRIORITY 20
#define SRXL_DEVICE_INFO    (SRXL_DEVINFO_NO_RF)
#define SRXL_SUPPORTED_BAUD_RATES   0
#define SRXL_BUS_IDX 0

#define SPEKTRUM_SRXL_MIN_LENGTH 5

struct srxl2_buffer {
    uint8_t raw_buffer[SRXL_MAX_BUFFER_SIZE];
    uint8_t cursor;
    bool ready;
};

struct srxl2_data {
    struct srxl2_buffer rx_buffers[2];
    uint8_t rx_buffer_idx;
    struct srxl2_buffer tx_buffer;
    uint8_t tx_id;
    int last_err;
    volatile bool received;
    struct rc_input last_val;
};

struct srxl2_config {
    const struct device *uart;
};

static void _init_data(struct srxl2_data *data) {
    data->rx_buffers[0].cursor = 0;
    data->rx_buffer_idx = 0;
    data->tx_buffer.ready = true;
    data->tx_id = 0;
    data->received = false;
}

struct srxl2_buffer *srxl2_get_active_buffer(const struct device *dev) {
    struct srxl2_data *data = dev->data;
    return &data->rx_buffers[data->rx_buffer_idx];
}

struct srxl2_buffer *srxl2_get_inactive_buffer(const struct device *dev) {
    struct srxl2_data *data = dev->data;
    return (data->rx_buffer_idx == 1 ? &data->rx_buffers[0] : &data->rx_buffers[1]);
}

static void srxl2_switch_buffer(const struct device *dev) {
    struct srxl2_data *data = dev->data;
    struct srxl2_buffer *active_buf = srxl2_get_active_buffer(dev);
    struct srxl2_buffer *inactive_buf = srxl2_get_inactive_buffer(dev);

    active_buf->ready = true;

    inactive_buf->ready = false;
    inactive_buf->cursor = 0;
   
    data->rx_buffer_idx = data->rx_buffer_idx == 0 ? 1 : 0;
}

static void srxl2_handle_rx(const struct device *dev) {
    struct srxl2_data *data = dev->data;
    const struct srxl2_config *config = dev->config;
    struct srxl2_buffer *active_buf = srxl2_get_active_buffer(dev);

    int ret = uart_fifo_read(config->uart, &(active_buf->raw_buffer[active_buf->cursor]), 1);
    active_buf->cursor++;
    if (ret < 0) {
        data->last_err = -ret;
    }

    // if first byte is not SPEKTRUM_SRXL_ID try again
    if (active_buf->cursor == 1 && active_buf->raw_buffer[0] != SPEKTRUM_SRXL_ID) {
        active_buf->cursor = 0;
        return;
    }

    // indicate, that we received something
    if(!data->received) {
        data->received = true;
    }

    // check whether message is complete
    if (active_buf->cursor >= SPEKTRUM_SRXL_MIN_LENGTH) {
        uint8_t length = active_buf->raw_buffer[2];

        // switch over when complete
        if (active_buf->cursor == length || active_buf->cursor > SRXL_MAX_BUFFER_SIZE) {
            srxl2_switch_buffer(dev);
        }
    }
}

static void srxl2_handle_tx(const struct device *dev) {
    struct srxl2_data *data = dev->data;
    const struct srxl2_config *config = dev->config;

    // reached end of buffer, stop
    if(data->tx_id >= data->tx_buffer.cursor-1) {
        data->tx_buffer.ready = true;
        uart_irq_tx_disable(config->uart);
    }

    int ret = uart_fifo_fill(config->uart, &data->tx_buffer.raw_buffer[data->tx_id], data->tx_buffer.cursor-data->tx_id);

    if (ret < 0) {
        data->last_err = ret;
        return;
    }

    // move write cursor forward
    data->tx_id += ret;
}

static void srxl2_cb_handler(const struct device *uart, void *dev) {
    uart_irq_update(uart);

    if (uart_irq_rx_ready(uart)) {
        srxl2_handle_rx((const struct device*)dev);
    }

    if (uart_irq_tx_ready(uart)) {
        srxl2_handle_tx((const struct device*)dev);
    }
}

static int srxl2_rc_init(const struct device *dev) {
    const struct srxl2_config *config = dev->config;

    if (!device_is_ready(config->uart)) {
        printk("Uart not ready.. !");
        return -EIO;
    }

    /* Init the local SRXL device */
    if(!srxlInitDevice(SRXL_DEVICE_ID, SRXL_DEVICE_PRIORITY, SRXL_DEVICE_INFO, 0xaba)) {
        printk("Failid initializing srxl dev\n");
        return -ENOTSUP;
    }

    /* Init the SRXL bus: The bus index must always be smaller than
       SRXL_NUM_OF_BUSES -- in this case, it can only be 0 */
    if(!srxlInitBus(SRXL_BUS_IDX, (void*)dev, SRXL_SUPPORTED_BAUD_RATES)) {
        printk("Failid initializing srxl bus\n");
        return -ENOTSUP;
    }

    _init_data(dev->data);

    /* Set and enable ISR for uart rx */
    uart_irq_rx_disable(config->uart);
	uart_irq_tx_disable(config->uart);
    uart_irq_callback_user_data_set(config->uart, srxl2_cb_handler, (void*)dev);
    uart_irq_rx_enable(config->uart);

    return 0;
}

/**
 * @brief Update SRXL2 State machine and parse available data
 * 
 * @param dev The receiver device
 * @param dt time passed since last call
 * @return int 0 or -error
 */
int rc_run_once(const struct device *dev, uint32_t dt) {
    struct srxl2_data *data = dev->data;
    const struct srxl2_config *config = dev->config;
    int ret = 0;

    /* Nothing received, update state machine with timeout time */
    if (!data->received) {
        //printk("Nothing received!\n");
        srxlRun(SRXL_BUS_IDX, (int16_t)dt);
        return 0;
    }

    struct srxl2_buffer *read_buffer = srxl2_get_inactive_buffer(dev);

    /* Disable rx interrupt while parsing buffer */
    uart_irq_rx_disable(config->uart);

    if (read_buffer->ready) {
        
        if(!srxlParsePacket(SRXL_BUS_IDX, read_buffer->raw_buffer, read_buffer->cursor)) {
            ret = -EINVAL;
        } else {
            data->last_val.roll = srxlChData.values[1];
            data->last_val.pitch = srxlChData.values[2];
            data->last_val.throttle = srxlChData.values[0];
            data->last_val.yaw = srxlChData.values[3];
        }

        read_buffer->ready = false;
    }

    /* Reenable rx interrupts again */
    uart_irq_rx_enable(config->uart);

    return ret;
}

void rc_update(const struct device *dev, struct rc_input *rc_in) {
    struct srxl2_data *data = dev->data;

    rc_in->roll = data->last_val.roll;
    rc_in->pitch = data->last_val.pitch;
    rc_in->throttle = data->last_val.throttle;
    rc_in->yaw = data->last_val.yaw;
}

void transmit_uart(void *dev_ptr, uint8_t * pBuffer, uint8_t length) {

    struct device *dev = (struct device*)dev_ptr;
    struct srxl2_data *data = dev->data;
    const struct srxl2_config *config = dev->config;

    if (length > SRXL_MAX_BUFFER_SIZE) {
        return;
    }

    // still sending, don't mix it up
    if (!data->tx_buffer.ready) {
        return;
    }
    
    memcpy(data->tx_buffer.raw_buffer, pBuffer, length);
    data->tx_buffer.cursor = length;
    data->tx_buffer.ready = false;

    uart_irq_tx_enable(config->uart);
}

#define SRXL2_RC_INIT(i) \
    static struct srxl2_data srxl2_data_##i; \
    static struct srxl2_config srxl2_config_##i = { \
        .uart = DEVICE_DT_GET(DT_INST_BUS(i)), \
    }; \
    DEVICE_DT_INST_DEFINE(i, &srxl2_rc_init, NULL, \
            &srxl2_data_##i, &srxl2_config_##i, \
            POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, \
            NULL);

DT_INST_FOREACH_STATUS_OKAY(SRXL2_RC_INIT)
/*

*/

