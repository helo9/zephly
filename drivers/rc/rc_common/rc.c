/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <math.h>
#include <custom_drivers/rc.h>
#include <settings/settings.h>

/* default settings */
#define RC_EXPO_ROLL_DEFAULT 0.0
#define RC_EXPO_PITCH_DEFAULT 0.0
#define RC_EXPO_YAW_DEFAULT 0.0

#define RC_SUPEREXPO_ROLL_DEFAULT 0.0
#define RC_SUPEREXPO_PITCH_DEFAULT 0.0
#define RC_SUPEREXPO_YAW_DEFAULT 0.0

/* settings variables */
static float rc_expo_roll = RC_EXPO_ROLL_DEFAULT;
static float rc_expo_pitch = RC_EXPO_PITCH_DEFAULT;
static float rc_expo_yaw = RC_EXPO_YAW_DEFAULT;
static float rc_superexpo_roll = RC_SUPEREXPO_ROLL_DEFAULT;
static float rc_superexpo_pitch = RC_SUPEREXPO_PITCH_DEFAULT;
static float rc_superexpo_yaw = RC_SUPEREXPO_YAW_DEFAULT;

static inline float clip(float in, float min_out, float max_out) {
    if (in < min_out) {
            return min_out; 
    } else if (in > max_out) { 
        return max_out; 
    } else {
        return in;
    }
}

float apply_superexpo(float val, float e, float g) {
    val = clip(val, -0.999, 0.999),
    e = clip(e, 0.0, 0.99);
    g = clip(g, 0.0, 1.0);
    float expo = (1.0f - e) * val + e * val * val * val;
    float superexpo = expo * (1.0f - g) / (1 - fabs(val) * g);

    return superexpo;
}

void rc_update_expo(const struct device *dev, struct Command *rc_in) {
    rc_update(dev, rc_in);

    rc_in->roll = apply_superexpo(rc_in->roll, rc_expo_roll, rc_superexpo_roll);
    rc_in->pitch = apply_superexpo(rc_in->pitch, rc_expo_pitch, rc_superexpo_pitch);
    rc_in->yaw = apply_superexpo(rc_in->yaw, rc_expo_yaw, rc_superexpo_yaw);
    rc_in->thrust = clip(rc_in->thrust, 0.0f, 1.0f);
}




