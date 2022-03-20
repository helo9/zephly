/*
* Copyright (c) 2022 Jonathan Hahn
* SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr.h>
#include <device.h>
#include <custom_drivers/rc.h>

#include <string.h>
#include <SDL.h>

#define DT_DRV_COMPAT zephly_joystickrc
#define JOYSTICK_HALF_RANGE 32768.0f

struct joystick_rc_data {
    SDL_Joystick *joystick0;
};

struct joystick_rc_config {};

static int joystick_rc_init(const struct device *dev) {
    struct joystick_rc_data *data = dev->data;

    if (SDL_Init(SDL_INIT_JOYSTICK) < 0 ) {
		return -ECANCELED;
	}

    if (SDL_NumJoysticks() < 1) {
        printk("No joystick available.\n");
        return -EIO;
    }

    data->joystick0 = SDL_JoystickOpen(0);

	if (data->joystick0 == NULL) {
        printk("Opening Joystick failed.\n");
		return -EIO;
	}

    const char * joystick_name = SDL_JoystickName(data->joystick0);

    printk("Connected with joystick: %s\n", joystick_name);

    if (strcmp(joystick_name, "OpenTX Jumper TLite Joystick") != 0) {
        printk("WARNING: Only tested for OpenTX Jumper TLite Joystick, you are using another joystick!!");
    }

    return 0;
}

static void joystick_rc_update(const struct device *dev, struct Command *rc_val) {
    struct joystick_rc_data *data = dev->data;

    if (data->joystick0 == NULL) {
        return;
    }

    SDL_JoystickUpdate();

    rc_val->roll   = SDL_JoystickGetAxis(data->joystick0, 0) / JOYSTICK_HALF_RANGE;
    rc_val->pitch  = -SDL_JoystickGetAxis(data->joystick0, 1) / JOYSTICK_HALF_RANGE;
    rc_val->thrust = 0.5 * (SDL_JoystickGetAxis(data->joystick0, 2) / JOYSTICK_HALF_RANGE + 1.0f);
    rc_val->yaw    = SDL_JoystickGetAxis(data->joystick0, 3) / JOYSTICK_HALF_RANGE;
    rc_val->armed  = true;
}

static struct joystick_rc_data data;

static const struct joystick_rc_config config;

struct rc_api joystick_rc_api = {
    .update = joystick_rc_update
};

DEVICE_DT_INST_DEFINE(0, &joystick_rc_init, NULL,
		      &data, &config,
		      POST_KERNEL, CONFIG_ZEPHLY_JOYSTICK_RC_INIT_PRIORITY,
		      &joystick_rc_api);
