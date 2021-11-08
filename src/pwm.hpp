#include <device.h>

#define PWM_DEVICE DT_NODELABEL(pwm1)

class PWM {

    static constexpr uint32_t _period = 20000; // 50 Hz update rate
    static constexpr uint32_t _min_duty = 1000; // 1ms minimal duty cycle
    static constexpr uint32_t _max_duty = 2000; // 2ms maximal duty cycle

public:
    PWM(PWM const&)		        = delete;
	void operator=(PWM const&)  = delete;

    static PWM& getInstance() {
        static PWM pwm(DEVICE_DT_GET(PWM_DEVICE));

        return pwm;
    }

    bool is_ready() const;
    
    void write(const float (&outputs)[4]);

private:
    PWM(const struct device* pwm_dev);

    const struct device* _pwm_dev;
};

constexpr auto pwm = PWM::getInstance;
