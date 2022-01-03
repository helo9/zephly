#include <zephyr.h>
#include "irq.h"
#include "kernel.h"
#include <device.h>
#include <stdio.h>
#include <devicetree.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <irq.h>

#include "rc/rc.hpp"

struct RCInput rc_input;

void main() {

    if (rc_init() < 0){
        printk("USART is not ready!");
        return;
    }

    while (true) {
        int ret = rc_update(5);
        rc_input = rc_get();
        
        printk("%d\t%d\t%d\t%d\n", rc_input.roll, rc_input.pitch, rc_input.throttle, rc_input.yaw);
        if(ret<0) {
            printk("ERROR %d\n", ret);
        }
        k_sleep(K_MSEC(5));
    }
}
