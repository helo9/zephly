#include "lowpass.h"

/**
 * @brief Initializes LowPass Filter
 * 
 * @param lp struct holding lowpass's data
 * @param f_s sampling frequency
 * @param f_c cutoff frequency
 */
void lowpass_init(struct LowPass *lp, float f_s, float f_c) {
    lp->alpha = f_c / (f_s + f_c);
    lp->last_y = 0.0f;
}

/**
 * @brief Calculate next filtered value
 * 
 * @param lp LowPass filter struct
 * @param x last input
 * @return float filtered value
 */
float lowpass_update(struct LowPass *lp, const float x) {
    lp->last_y = (1-lp->alpha) * lp->last_y + lp->alpha * x;

    return lp->last_y;
}

/**
 * @brief reset low pass filter
 * 
 * @param lp LowPass filter struct
 */
void lopwass_reset(struct LowPass *lp) {
    lp->last_y = 0.0f;
}
