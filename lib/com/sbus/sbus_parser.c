/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/util.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <sbus_parser.h>

#define SBUS_START_BYTE 0x0F
#define SBUS_END_BYTE 0x00
#define SBUS_SERVO_CHANNEL_BIT_LEN 11

#define SBUS_FLAGS_DIG_CHANNEL_0_MASK 0x01
#define SBUS_FLAGS_DIG_CHANNEL_1_MASK 0x02
#define SBUS_FLAGS_FRAME_LOST_MASK 0x04
#define SBUS_FLAGS_FAILSAFE_MASK 0x08

static inline bool is_byte_valid(uint8_t byte, int pointer) {
    switch (pointer) {
    case 0:
        return byte == SBUS_START_BYTE;
    case SBUS_RECV_BUFFER_SIZE-1:
        return byte == SBUS_END_BYTE;
    default:
        return true;
    }
}

static inline void reset_buffer(struct SBusBuffer *buffer) {
    buffer->pointer = 0;
}

bool sbus_is_buffer_full(struct SBusBuffer *buffer) {
    return buffer->pointer >= SBUS_RECV_BUFFER_SIZE;
}

int sbus_add_to_buffer(struct SBusBuffer *buffer, const uint8_t byte) {

    if (sbus_is_buffer_full(buffer)) {
        return -ENOBUFS;
    }

    if (!is_byte_valid(byte, buffer->pointer)) {
        reset_buffer(buffer);
        return -EINVAL;
    }

    buffer->data[buffer->pointer] = byte;
    buffer->pointer++;

    return 0;
}

static uint16_t sbus_read_next_servo_chan(struct SBusParsingCursor *cursor, const uint8_t *raw_buffer) {
    uint16_t servo_chan_value = 0;
    int bits_read = 0;

    while (bits_read < SBUS_SERVO_CHANNEL_BIT_LEN) {
        const int bits_missing = SBUS_SERVO_CHANNEL_BIT_LEN - bits_read;
        const int bits_left_in_byte = 8 - cursor->bit_pos;
        const int bits_to_read = MIN(bits_missing, bits_left_in_byte);
        const uint8_t bit_mask = pow(2, bits_to_read) - 1;
        const uint8_t bits_from_buffer = (raw_buffer[cursor->byte_pos] >> cursor->bit_pos) & bit_mask;

        servo_chan_value |= (uint16_t)bits_from_buffer << bits_read;

        bits_read += bits_to_read;
        cursor->bit_pos += bits_to_read;

        if(cursor->bit_pos == 8) {
            cursor->byte_pos++;
            cursor->bit_pos = 0;
        }
    }

    return servo_chan_value;
}

static void sbus_parse_flags_byte(uint8_t flags_byte, struct SBusData *data) {
    data->digital_channels[0] = flags_byte & SBUS_FLAGS_DIG_CHANNEL_0_MASK;
    data->digital_channels[1] = flags_byte & SBUS_FLAGS_DIG_CHANNEL_1_MASK;
    data->frame_lost = flags_byte & SBUS_FLAGS_FRAME_LOST_MASK;
    data->failsafe_active = flags_byte & SBUS_FLAGS_FAILSAFE_MASK;
}

int sbus_parse_buffer(struct SBusBuffer *buffer, struct SBusData *data) {

    if (buffer->pointer != SBUS_RECV_BUFFER_SIZE) {
        return -EINVAL;
    }

    struct SBusParsingCursor buffer_cursor = {};
    const uint8_t *raw_data_buffer = &buffer->data[1];

    for (int servo_chan=0; servo_chan<SBUS_NUM_SERVO_CHANNELS; servo_chan++) {
        
        data->servo_channels[servo_chan] = sbus_read_next_servo_chan(&buffer_cursor, raw_data_buffer);
    }

    const uint8_t flags_byte_position = SBUS_RECV_BUFFER_SIZE - 2;
    const uint8_t flags_byte = buffer->data[flags_byte_position];
    sbus_parse_flags_byte(flags_byte, data);

    reset_buffer(buffer);

    return 0;
}
