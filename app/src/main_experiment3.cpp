#include <zephyr.h>
#include "irq.h"
#include "kernel.h"
#include <device.h>
#include <stdio.h>
#include <devicetree.h>

void test_work_handler(k_work *work) {
    printk("Test so und so\n");
}

K_WORK_DEFINE(test_work, test_work_handler);

void test_timer_handler(k_timer *dummy) {
    k_work_submit(&test_work);
}

K_TIMER_DEFINE(test_timer, test_timer_handler, NULL);

void main() {

    k_timer_start(&test_timer, K_MSEC(5), K_MSEC(5));

    while (true) {
        
        printk("noch da!\n");

        k_sleep(K_MSEC(1000));
    }
}
