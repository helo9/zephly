/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SBUS_PARSER_H
#define SBUS_PARSER_H

#include <stdbool.h>
#include <stdint.h>

#define SBUS_RECV_BUFFER_SIZE 25
#define SBUS_NUM_SERVO_CHANNELS 16
#define SBUS_NUM_DIGI_CHANNELS 2

struct SBusBuffer {
    uint8_t pointer;
    uint8_t data[SBUS_RECV_BUFFER_SIZE];
};

struct SBusData {
    uint16_t servo_channels[SBUS_NUM_SERVO_CHANNELS];
    bool digital_channels[SBUS_NUM_DIGI_CHANNELS];
    bool frame_lost;
    bool failsafe_active;
};

struct SBusParsingCursor {
    int byte_pos;
    int bit_pos;
};

/**
 * @brief Check if the parser's buffer is full
 * 
 * @param parser the instance
 * @return true full
 * @return false not full
 */
bool sbus_is_buffer_full(struct SBusBuffer *buffer);

/**
 * @brief Add byte into the parser's buffer
 * 
 * @param parser the instance
 * @param byte byte to append
 * @return int 0 when succesful, -errno otherwise
 */
int sbus_add_to_buffer(struct SBusBuffer *buffer, const uint8_t byte);

/**
 * @brief Parse the parser's internal buffer
 * 
 * @param parser the instance
 * @param data extracted data
 * @return int 0 when succesful, -errno otherwise
 */
int sbus_parse_buffer(struct SBusBuffer *buffer, struct SBusData *data);

#endif /* SBUS_PARSER_H */
