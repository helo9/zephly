/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "lowpass.h"

#ifndef FFC_LIB_RATECONTROL_H
#define FFC_LIB_RATECONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

enum SATURATION_STATE {
    NO_SATURATION,
    POSITIVE_SATURATION,
    NEGATIVE_SATURATION
};

struct RateControl {
    float k_p;
    float k_i;
    float k_d;

    float dt;

    float i_sum;
    float i_lim_upper;
    float i_lim_lower;

    struct LowPass d_lpf;
    float last_y;
    bool d_ready;

    float p_val;
    float i_val;
    float d_val;

    enum SATURATION_STATE saturation_state;
};

void ratecontrol_init(struct RateControl *ratecontrol, const char *settings_prefix);

float ratecontrol_update(struct RateControl *ratecontrol, float sp, float y);

void ratecontrol_reset(struct RateControl *ratecontrol);

void ratecontrol_backcalculation(struct RateControl *ratecontrol, float o_des);

void ratecontrol_set_saturation(struct RateControl *ratecontrol, enum SATURATION_STATE state);

#ifdef __cplusplus
}
#endif 

#endif /* FFC_LIB_RATECONTROL_H */
