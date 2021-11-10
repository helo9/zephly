#include "pid.hpp"
#include "math.h"

#include <iostream>

PID::PID(float K, float P, float I, float D) 
    : _K(K), _P(P), _I(I), _D(D) {}

float clip(float x, float limit) {
    if (x > limit) {
        return limit;
    } else if (x < -limit) {
        return -limit;
    } else{
        return x;
    }
}

float PID::update(const float x, const float tar, const float dt_ms) {
    float p = 0.0f, i = 0.0f, d = 0.0f, out=0.0f;

    if (_initialized) {
        // prevent deivions by zero
        if (dt_ms*dt_ms > 0.0f) {
            d = -_D * (x - _last_x) / dt_ms;
        }

        float err = tar - x;

        p = _P * err;

        if (_integral > -_INTEGRAL_LIMIT && _integral < _INTEGRAL_LIMIT) {
            _integral += err * dt_ms;
        }

        _integral = clip(_integral, _INTEGRAL_LIMIT);

        i = _I * _integral;

        out = _K * (p + i + d);
    } else {
        _initialized = true;
    }

    _last_x = x;

    return out;
}

void PID::reset() {
    _last_x = 0.0f;
    _integral = 0.0f;
    _initialized = false;
}
