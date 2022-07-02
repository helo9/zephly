/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHLY_SIMULATOR_SOCKET
#define ZEPHLY_SIMULATOR_SOCKET

#include <stdint.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

struct simulator_socket {
    int sockfd;
    struct sockaddr_in servaddr;
};

int simulator_socket_initialize(struct simulator_socket *data);

int simulator_socket_send(const struct simulator_socket *data, uint8_t *buf, size_t buf_len);

int simulator_socket_receive(struct simulator_socket *data, uint8_t *buf, size_t buf_len);

#endif /* ZEPHLY_SIMULATOR_SOCKET */
