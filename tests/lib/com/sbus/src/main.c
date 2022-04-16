/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>

#include <sbus_parser.h>

#define BITS_IN_BYTE 8
#define BITS_PER_SBUS_CHAN 11

uint16_t sbus_read_next_servo_chan(struct SBusParsingCursor *parser_pointer, const uint8_t *raw_buffer);

void test_read_11bit_servo_value_from_buffer_start(void) {
    
    // Setup
    
    uint8_t buffer[SBUS_RECV_BUFFER_SIZE] = {};

    buffer[0] = 0xFF;
    buffer[1] = 0xFF;

    struct SBusParsingCursor buffer_pos = {
        .bit_pos = 0, .byte_pos = 0
    };

    // Action

    uint16_t value = sbus_read_next_servo_chan(&buffer_pos, buffer);

    // Checks

    zassert_equal(value, 0b11111111111, "The buffer contained only ones, but we read %x instead of 0x7FF", value);
}

void test_read_11bit_servo_value_from_mid_buffer(void) {

    // Setup

    uint8_t buffer[SBUS_RECV_BUFFER_SIZE] = {};

    buffer[1] = 0xFF;
    buffer[2] = 0xAF;

    struct SBusParsingCursor buffer_pos = {
        .byte_pos = 1,
        .bit_pos = 7
    };

    // Action

    uint16_t value = sbus_read_next_servo_chan(&buffer_pos, buffer);

    // Checks

    zassert_equal(value, 0x15F, "Read Value is incorrect, %d.", value);
}

void sbus_parse_flags_byte(uint8_t flags_byte, struct SBusData *data);

void test_flag_parsing(void) {
    
    // Setup

    struct SBusData data = {};
    uint8_t flags_byte = 0xBA;

    // Action

    sbus_parse_flags_byte(flags_byte, &data);

    // Checks

    zassert_equal(data.digital_channels[0], false, "First digital channel has wrong value.");
    zassert_equal(data.digital_channels[1], true, "Second digital channel has wrong value.");
    zassert_equal(data.failsafe_active, true, "Failsafe has wrong value.");
    zassert_equal(data.frame_lost, false, "Frame lost has wrong value.");
}

void test_parse_invalid_start_byte(void) {
    struct SBusBuffer buffer = {};

    const uint8_t invalid_start_byte = 0x00;

    int ret = sbus_add_to_buffer(&buffer, invalid_start_byte);

    zassert_equal(ret, -EINVAL, "Wrong start byte was not detected.");
}

static void fill_parser_buffer(struct SBusBuffer *buffer, const uint8_t *input, const uint8_t num) {
    for (int i=0; i<num; i++) {
        int ret = sbus_add_to_buffer(buffer, input[i]);

        zassert_equal(ret, 0, "Valid data was rejected.");
    }
}

void test_feed_in_invalid_end_byte(void) {

    // Setup

    struct SBusBuffer buffer = {};

    const uint8_t valid_start_byte = 0xF0, invalid_end_byte = 0xFF;

    uint8_t input[SBUS_RECV_BUFFER_SIZE] = {};
    input[0] = valid_start_byte;

    // Actions

    fill_parser_buffer(&buffer, input, SBUS_RECV_BUFFER_SIZE-1);

    int ret = sbus_add_to_buffer(&buffer, invalid_end_byte);

    // Checks

    zassert_equal(ret, -EINVAL, "Wrong end byte was not detected.");
}

void test_feed_in_too_much_data(void) {

    // Setup

    struct SBusBuffer buffer = {};

    const uint8_t valid_start_byte = 0xF0, valid_end_byte = 0x00;

    uint8_t input[SBUS_RECV_BUFFER_SIZE] = {};
    input[0] = valid_start_byte;
    input[SBUS_RECV_BUFFER_SIZE-1] = valid_end_byte;

    // Actions

    fill_parser_buffer(&buffer, input, SBUS_RECV_BUFFER_SIZE);

    int ret = sbus_add_to_buffer(&buffer, 0xA0);

    // Checks

    zassert_equal(ret, -ENOBUFS, "Buffer overflow was not detected.");
}

void test_complete_parsing(void) {

    // Setup

    struct SBusBuffer buffer = {};

    const uint8_t valid_start_byte = 0xF0, valid_end_byte = 0x00;

    const uint8_t input[SBUS_RECV_BUFFER_SIZE] = {
        valid_start_byte, 0x00, 0xB3, 0x8A, 0x77,
        0x64, 0xEA, 0xA0, 0x61, 0x02,
        0xFA, 0x00, 0x23, 0x8A, 0x73,
        0x64, 0xEA, 0x00, 0x61, 0x02,
        0x53, 0xEE, 0xAA, 0xBA, valid_end_byte
    };

    const uint16_t expected_values[SBUS_NUM_SERVO_CHANNELS] = {
        768, 342, 478, 1330, 526, 1219, 1664, 7, 547, 1649, 401,
        117, 1552, 1540, 916, 1367
    };

    fill_parser_buffer(&buffer, input, SBUS_RECV_BUFFER_SIZE);

    struct SBusData data;

    // Call

    int ret = sbus_parse_buffer(&buffer,&data);

    // Checks

    zassert_equal(ret, 0, "Parsing failed.");

    for (int i=0; i<SBUS_NUM_SERVO_CHANNELS; i++) {

        zassert_equal(data.servo_channels[i], expected_values[i], "Extracted invalid data.");
        
    }
}

void test_main(void) {
    ztest_test_suite(sbus_tests,
        ztest_unit_test(test_read_11bit_servo_value_from_buffer_start),
        ztest_unit_test(test_read_11bit_servo_value_from_mid_buffer),
        ztest_unit_test(test_flag_parsing),
        ztest_unit_test(test_parse_invalid_start_byte),
        ztest_unit_test(test_feed_in_invalid_end_byte),
        ztest_unit_test(test_feed_in_too_much_data),
        ztest_unit_test(test_complete_parsing)
    );

    ztest_run_test_suite(sbus_tests);
}
