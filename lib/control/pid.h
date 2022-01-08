#include <stdbool.h>

#ifndef FFC_LIB_PID_H
#define FFC_LIB_PID_H

struct PID {
    float k;
    float p;
    float i;
    float d;
    float last_x;
    float ival;
    bool initialized;
    float dt;
    float ilim;
};

void pid_init(struct PID *pid, float k, float p, float i, float d, float dt, float ilim);
float pid_update(struct PID *pid, const float x, const float tar);
void pid_reset(struct PID *pid);

#endif // FFC_LIB_PID_H
