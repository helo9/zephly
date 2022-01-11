#include <zephyr.h>
#include <device.h>
#include <devicetree.h>

#include "custom_drivers/rc.h"

#define RC_IN DT_NODELABEL(rc)

const struct device *rc = DEVICE_DT_GET(RC_IN);
struct rc_input rc_in;

void main() {

    printk("Hallo Welt\n");

    while (true)  {

        rc_update(rc, &rc_in);
        printk("Noch da :)\n");
        printk("%d %d %d %d\n", rc_in.roll, rc_in.pitch, rc_in.throttle, rc_in.yaw);

        k_sleep(K_MSEC(500));
    }
}
