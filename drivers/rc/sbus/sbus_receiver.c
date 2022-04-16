/*
* Copyright (c) 2022 Jonathan Hahn
*
* SPDX-License-Identifier: Apache-2.0
*/

#include "devicetree.h"
#include <zephyr.h>
#include <device.h>
#include <custom_drivers/rc.h>

#define DT_DRV_COMPAT futaba_sbusreceiver

struct sbus_data {
    
};
struct sbus_config {
    const struct device *uart;
};

static int sbus_receiver_init(const struct device *dev) {
    const struct sbus_config *config = dev->config;

    if (!device_is_ready(config->uart)) {
        printk("UART not ready.");
        return -EIO;
    }
    
    return 0;
}

static void sbus_update(const struct device *dev, struct Command *rc_val) {
    return;
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
