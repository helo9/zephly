/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHLY_SIMULATOR_NETWORK
#define ZEPHLY_SIMULATOR_NETWORK

#include <stdint.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

struct simulator_network_data {
    int sockfd;
    struct sockaddr_in servaddr;
};

int simulator_initialize_network(struct simulator_network_data *data);

int simulator_send(const struct simulator_network_data *data, uint8_t *buf, size_t buf_len);

int simulator_receive(struct simulator_network_data *data, uint8_t *buf, size_t buf_len);

#endif /* ZEPHLY_SIMULATOR_NETWORK */
