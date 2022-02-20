
/* main.c - Sample sending gyro measurements via UART to host. */

/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kernel.h"
#include <zephyr.h>
#include <devicetree.h>
#include <drivers/uart.h>
#include <drivers/sensor.h>
#include <stdint.h>
#include <gyro_msg.h>

#define SYNC_BYTE 0xAA

#define CONSUMER_STACK_SIZE 500
#define CONSUMER_STACK_PRIO 5

K_MSGQ_DEFINE(measurement_queue, sizeof(union measurement_msg), 10, 4);

static inline int32_t sensor_value_to_int(const struct sensor_value *val) {
    return (int32_t)(val->val1 * 10000) + (int32_t)(val->val2 / 100);
}

/* Consumer Thread with msg queue */

extern void consumer_run(void *, void *, void *);

K_THREAD_DEFINE(my_tid, CONSUMER_STACK_SIZE,
            consumer_run, NULL, NULL, NULL,
            CONSUMER_STACK_PRIO, 0, 0);

/* Producer runs with timer and regularly catches sensor measurements */

void produce_measurement(struct k_work *work);

K_WORK_DEFINE(measurement_work, produce_measurement);

void measurement_timer_handler(struct k_timer *timer_id);

K_TIMER_DEFINE(producer_timer, measurement_timer_handler, NULL);

/* Devieces */

#define UART_NODE DT_NODELABEL(usart1)
#define GYRO_NODE DT_NODELABEL(imu)

const struct device *uart = DEVICE_DT_GET(UART_NODE);
const struct device *gyro = DEVICE_DT_GET(GYRO_NODE);

void main() {

    printk("Starting initialization");

    if (!device_is_ready(gyro)) {
        printk("Gyro is not ready!");
        return;
    }

    printk("..");

    if (!device_is_ready(uart)) {
        printk("Uart is not ready!");
        return;
    }

    printk("..");

    k_timer_start(&producer_timer, K_MSEC(1), K_MSEC(1));

    printk("done\n");


    while (true) {
        //printk("Hello my dear friend ;)\n");
        k_sleep(K_MSEC(5000));
    }
}

extern void consumer_run(void *_, void *__, void *___)
{
    union measurement_msg data = {};

    while (true) {
        k_msgq_get(&measurement_queue, &data, K_FOREVER);

        const int len = sizeof(data);

        uart_poll_out(uart, (char)SYNC_BYTE);
        uart_poll_out(uart, (char)SYNC_BYTE);
        uart_poll_out(uart, (char)len);
        for (int i=0; i<len; i++) {
            uart_poll_out(uart, data.raw[i]);
        }
    }
}

void measurement_timer_handler(struct k_timer *timer) {
    k_work_submit(&measurement_work);
}

void produce_measurement(struct k_work *work) {
    struct sensor_value measurements[3];
    sensor_sample_fetch(gyro);
    sensor_channel_get(gyro, SENSOR_CHAN_GYRO_XYZ, measurements);

    union measurement_msg data = {
        .values = {
            (uint16_t)k_uptime_get(),
            sensor_value_to_int(&measurements[0]),
            sensor_value_to_int(&measurements[1]),
            sensor_value_to_int(&measurements[2])
        }
    };

    k_msgq_put(&measurement_queue, &data, K_NO_WAIT);
}
