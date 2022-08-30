#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <shell/shell.h>
#include <settings/settings.h>

#include "custom_drivers/rc.h"
#include "sys/util.h"

#define RC_IN DT_NODELABEL(rc)

const struct device *receiver = DEVICE_DT_GET(RC_IN);
struct Command rc_in;

static volatile bool do_print = true;

#if IS_ENABLED(CONFIG_SETTINGS)

static int cmd_save_settings(const struct shell *sh, size_t argc, char **argv) {
    settings_save();
    return 0;
}

SHELL_CMD_REGISTER(settings_save, NULL, "Save all parameters", cmd_save_settings);

#endif 

static int cmd_toggle_print(const struct shell *sh, size_t argc, char **argv) {
    do_print = !do_print;
    return 0;
}

SHELL_CMD_REGISTER(toggle_printing, NULL, "Toggle rc value print", cmd_toggle_print);

const uint8_t buf[25] = {0xFF, 0xE3, 0x83, 0xDE, 0x49, 0xB6, 0xC7, 0x0A, 0xF0, 0x81, 0x0F, 0x7C, 0xE0, 0x03, 0x1F, 0xF8, 0xC0, 0x07, 0x3E, 0xF0, 0x81, 0x4F, 0xCB, 0x00, 0x00};

void main() {

    // check for rc device
	if (!device_is_ready(receiver)) {
		printk("RC IN device not ready.\n");
		return;
	}

    int rc;

#if IS_ENABLED(CONFIG_SETTINGS)

    // check and initialize seetings
    rc = settings_subsys_init();
	if (rc) {
		printk("settings subsys initialization: fail (err %d)\n", rc);
		return;
	}

    rc = settings_load();
    if (rc) {
        printk("Failed loading settings from none volatile storage. Failed with error %d\n", rc);
    }
#endif 

    while (true)  {
        rc_update(receiver, &rc_in);
        if (do_print) {
            printk("Got roll: %d, pitch: %d, thrust: %d, yaw: %d, armed: %d\n", 
                (int)(rc_in.roll*100), (int)(rc_in.pitch*100), 
                (int)(rc_in.thrust*100), (int)(rc_in.yaw*100),
                (int)(rc_in.armed));
        }
        
        k_sleep(K_MSEC(200));
    }
}
