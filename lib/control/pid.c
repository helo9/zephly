#include "pid.h"

float _pid_clip(float val, float lim) {
    if (val > lim) {
        return lim;
    } else if (val < -lim) {
        return -lim;
    } else {
        return val;
    }
}

/**
 * @brief Initialize PID struct
 * 
 * @param pid the struct to be initialized
 * @param k serial gain (set to 1.0f if p!=1.0f)
 * @param p parallel gain (set to 1.0f if k!=1.0f)
 * @param i integral gain
 * @param d differential gain
 * @param dt sample time
 * @param ilim integrator limit
 */
void pid_init(struct PID *pid, float k, float p, float i, float d, float dt, float ilim) {
    pid->k = k;
    pid->p = p;
    pid->i = i;
    pid->d = d;
    pid->last_x = 0.0f;
    pid->ival = 0.0f;
    pid->initialized = false;
    pid->dt = dt;
    pid->ilim = ilim;
}

/**
 * @brief calculate new output based on measurement x and sp
 * 
 * @param pid the PID struct
 * @param x the last measurment
 * @param sp the setpoint 
 * @return float new output value 
 */
float pid_update(struct PID *pid, const float x, const float sp) {
    float out = 0.0f;

    if (pid->initialized) {
        float err = sp - x;

        float diff = (x - pid->last_x) / pid->dt;

        if ((pid->ival > -pid->ilim) || (pid->ival < pid->ilim)) {
            pid->ival = _pid_clip(pid->ival + err * pid->dt, pid->ilim);
        }
        
        out =  pid->p * err + pid->i * pid->ival - pid->d * diff;
        out *= pid->k;
    } else {
        pid->initialized = true;
    }

    pid->last_x = x;

    return  out;
}

/**
 * @brief reset pid controller
 * 
 * @param pid the PID struct to reset
 */
void pid_reset(struct PID *pid) {
    pid->last_x = 0.0f;
    pid->ival = 0.0f;
    pid->initialized = false;
}
