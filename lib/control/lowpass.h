#include <stdbool.h>

#ifndef FFC_LIB_LOWPASS_H
#define FFC_LIB_LOWPASS_H

struct LowPass {
    float alpha;
    float last_y;
};

void lowpass_init(struct LowPass *lp, float f_s, float f_c);
float lowpass_update(struct LowPass *lp, const float x);
void lowpass_reset(struct LowPass *lp);


#endif // FFC_LIB_LOWPASS_H
