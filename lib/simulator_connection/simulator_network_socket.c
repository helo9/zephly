/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <unistd.h>

#include "simulator_network_socket.h"

#define SIMULATOR_IP "127.0.0.1"
#define SIMULATOR_PORT    14560

static void simulator_initialize_sockaddr(struct sockaddr_in *addr) {
    memset(addr, 0, sizeof(*addr));

	/* Filling server information */
    addr->sin_family = AF_INET;
    addr->sin_port = htons(SIMULATOR_PORT);
    addr->sin_addr.s_addr = inet_addr(SIMULATOR_IP);
}

int simulator_socket_initialize(struct simulator_socket *data) {

    simulator_initialize_sockaddr(&data->servaddr);

    data->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (data->sockfd < 0) {
        return data->sockfd;
    }

    return 0;
}

int simulator_socket_send(const struct simulator_socket *data, uint8_t *buf, size_t buf_len) {
    return sendto(data->sockfd, (const char*)buf, buf_len,
        MSG_CONFIRM, (const struct sockaddr *)&data->servaddr,
        sizeof(data->servaddr));
}

int simulator_socket_receive(struct simulator_socket *data, uint8_t *buf, size_t buf_len) {
    struct sockaddr servaddr;
    int servaddr_len;

    int n = recvfrom(data->sockfd, (char *)buf, buf_len, 
        MSG_DONTWAIT, (struct sockaddr *) &servaddr,
        &servaddr_len);

    return n;
}
