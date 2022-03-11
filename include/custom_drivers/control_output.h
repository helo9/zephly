/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHLY_CONTROL_OUTPUT_H
#define ZEPHLY_CONTROL_OUTPUT_H

#include <device.h>

struct control_output_api {
    int (*set)(const struct device *dev, const float outputs[4]);
};

static inline int control_output_set(const struct device *dev, const float outputs[4]) {
    const struct control_output_api *api = (const struct control_output_api*)dev->api;
    return api->set(dev, outputs);
}

static inline int control_output_disable_motors(const struct device *dev) {
    static const float zero_output[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    return control_output_set(dev, zero_output);
}

#endif /* ZEPHLY_CONTROL_OUTPUT_H */
