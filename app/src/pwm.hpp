#include <device.h>


class PWM {

    static constexpr uint32_t _period = 20000; // 50 Hz update rate
    static constexpr uint32_t _min_duty = 1000; // 1ms minimal duty cycle
    static constexpr uint32_t _max_duty = 2000; // 2ms maximal duty cycle

public:
    PWM(PWM const&)		        = delete;
	void operator=(PWM const&)  = delete;

    static PWM& init() {
        static PWM pwm;

        return pwm;
    }

    bool is_ready() const;

    uint32_t _get_pulse(float setpoint) const;
    
    void write(const float (&outputs)[4]);

    void print();

private:
    PWM();

    const struct device* _pwm_devs[4];
    uint8_t _pwm_num_of_pins[4];

};
