#include <zephyr.h>
#include <device.h>
#include <devicetree.h>

#include "custom_drivers/rc.h"

#define RC_IN DT_NODELABEL(rc)

const struct device *rc = DEVICE_DT_GET(RC_IN);
struct rc_channels rc_in;

void main() {

	if (!device_is_ready(rc)) {
		printk("RC IN device not ready.\n");
		return;
	}

    while (true)  {
        rc_update(rc, &rc_in);
        printk("Got roll: %d, pitch: %d, thrust: %d, yaw: %d\n", 
            (int)(rc_in.roll*100), (int)(rc_in.pitch*100), 
            (int)(rc_in.throttle*100), (int)(rc_in.yaw*100));

        k_sleep(K_MSEC(500));
    }
}
