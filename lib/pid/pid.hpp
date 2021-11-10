#ifndef LIB_PID_H
#define LIB_PID_H

/* 
 * PID controller implementation
 */
class PID {
    static constexpr float _INTEGRAL_LIMIT = 10.0f;
public:
    PID(float K, float P, float I, float D);
    ~PID() = default;

    float update(const float x, const float tar, const float dt_ms);

    float get_integral_value() {return _integral;};

    void reset();
private:
    // proportional gain
    float _K;

    // parallel proportianal gain
    float _P;

    // integral gain
    float _I;

    // differential gain
    float _D;

    float _last_x = 0.0f;

    float _integral = 0.0f;

    bool _initialized = false;
};

#endif // LIB_PID_H
