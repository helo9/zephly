#include <zephyr.h>
#include <device.h>
#include <devicetree.h>

#include "custom_drivers/rc.h"

#define RC_IN DT_NODELABEL(rc)

const struct device *rc = DEVICE_DT_GET(RC_IN);
struct rc_input rc_in;

void main() {

    printk("Hallo Welt\n");

    unsigned int counter = 0;
    int err = 0;
    while (true)  {
        if (counter++ > 400) {
            rc_update(rc, &rc_in);
            printk("Noch da :) (%d)\n", err);
            printk("%d %d %d %d\n", rc_in.roll, rc_in.pitch, rc_in.throttle, rc_in.yaw);
            counter = 0;
        }

        err = rc_run_once(rc, 5);

        k_sleep(K_MSEC(5));
    }
}
