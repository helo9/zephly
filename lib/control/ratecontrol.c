#include "ratecontrol.h"

static const float k_p_default = 5.0f;
static const float k_i_default = 1.0f;
static const float k_d_default = 0.4f;

static const float i_lim = 15.0f;

static const float dt = 0.01f;
static const float f_c = 400.0f;

static float _calc_d(struct RateControl *ratecontrol, float y);
static void _update_integrator(struct RateControl *ratecontrol, float err);

static inline float clip(float in, float min_out, float max_out) {
    if (in < min_out) {
            return min_out; 
    } else if (in > max_out) { 
        return max_out; 
    } else {
        return in;
    }
}

void ratecontrol_init(struct RateControl *ratecontrol, const char *settings_prefix) {
    ratecontrol->k_p = k_p_default;
    ratecontrol->k_i = k_i_default;
    ratecontrol->k_d = k_d_default;

    ratecontrol->dt = dt;

    ratecontrol->i_sum = 0.0f;
    ratecontrol->i_lim_lower = -i_lim;
    ratecontrol->i_lim_upper = i_lim;

    lowpass_init(&ratecontrol->d_lpf, 1/dt, f_c);

    ratecontrol->last_y = 0.0f;

    ratecontrol->p_val = 0.0f;
    ratecontrol->i_val = 0.0f;
    ratecontrol->d_val = 0.0f;

    ratecontrol->saturation_state = NO_SATURATION;

    ratecontrol->d_ready = false;
}

float ratecontrol_update(struct RateControl *ratecontrol, float sp, float y) {
    float err = sp - y;
    
    ratecontrol->p_val = ratecontrol->k_p * err;
    ratecontrol->i_val = ratecontrol->k_i * ratecontrol->i_sum;
    ratecontrol->d_val = -ratecontrol->k_d * _calc_d(ratecontrol, y);

    _update_integrator(ratecontrol, err);

    return ratecontrol->p_val + ratecontrol->i_val + ratecontrol->d_val;
}

void ratecontrol_reset(struct RateControl *ratecontrol) {
    ratecontrol->last_y = 0.0f;
    ratecontrol->d_ready = false;

    ratecontrol->i_sum = 0.0f;
    
    lowpass_reset(&ratecontrol->d_lpf);

    /* p_val, i_val and d_val are not reset, because they aren't affecting
        following calculations */
}

void ratecontrol_backcalculation(struct RateControl *ratecontrol, float o_des) {
    ratecontrol->i_sum = (o_des - ratecontrol->p_val - ratecontrol->d_val) / ratecontrol->k_i;
}

void ratecontrol_set_saturation(struct RateControl *ratecontrol, enum SATURATION_STATE state) {
    ratecontrol->saturation_state = state;    
}

static float _calc_d(struct RateControl *ratecontrol, float y) {
    
    float dif;
    if (ratecontrol->d_ready) {
        dif = (y - ratecontrol->last_y) / ratecontrol->dt;
    } else {
        dif = 0.0f;
        ratecontrol->d_ready = true;
    }

    ratecontrol->last_y = y;

    return lowpass_update(&ratecontrol->d_lpf, dif);
}

static void _update_integrator(struct RateControl *ratecontrol, float err) {
    /* Limit error in case saturation was detected too stop further grow in case of staturation */
    if ((ratecontrol->saturation_state == POSITIVE_SATURATION  && err > 0.0f)
            || (ratecontrol->saturation_state == NEGATIVE_SATURATION  && err < 0.0f))
    {
        err = 0.0f;
    }

    ratecontrol->i_sum += err * ratecontrol->dt;

    ratecontrol->i_sum = clip(ratecontrol->i_sum, ratecontrol->i_lim_lower, ratecontrol->i_lim_upper);
}
