#include <zephyr.h>
#include <device.h>
#include <irq.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>
#include <string.h>
#include "rc.hpp"

#define RC_TIMER_EXPIRATION_MS (5)

void rc_work_handler(struct k_work *work) {
    rc_update(RC_TIMER_EXPIRATION_MS);
}

K_WORK_DEFINE(rc_work, rc_work_handler);

void rc_timer_handler(struct k_timer *dummpy) {
    k_work_submit(&rc_work);
}

K_TIMER_DEFINE(rc_timer, rc_timer_handler, NULL);

void rc_run() {
    k_timer_start(&rc_timer, K_MSEC(RC_TIMER_EXPIRATION_MS), K_MSEC(RC_TIMER_EXPIRATION_MS));
}
