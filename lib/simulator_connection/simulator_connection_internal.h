/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHLY_SIMULATOR_INTERNAL_H
#define ZEPHLY_SIMULATOR_INTERNAL_H

#include <simulator_connection.h>
#include "simulator_network_socket.h"

struct simulator_data {
    struct simulator_socket sim_socket;
    bool ready;
};

#endif /* ZEPHLY_SIMULATOR_INTERNAL_H */
