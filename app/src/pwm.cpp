#include "pwm.hpp"
#include <zephyr.h>
#include <drivers/pwm.h>
#include <devicetree.h>

#define NODE_OKAY(node) DT_NODE_HAS_STATUS(DT_NODELABEL(node), okay)
#define GET_PWM(node) DEVICE_DT_GET(DT_NODELABEL(node))
#define GET_PWM_NUM_PINS(node) 0

PWM::PWM() {
    _pwm_devs[0] = GET_PWM(pwm1);
    _pwm_devs[1] = GET_PWM(pwm2);
};

bool PWM::is_ready() const {
    return device_is_ready(_pwm_devs[0]) && device_is_ready(_pwm_devs[1]);
}

uint32_t PWM::_get_pulse(float setpoint) const {
    return _min_duty + static_cast<uint32_t>( (_max_duty - _min_duty) * setpoint);
}

void PWM::write(const float (&outputs)[4]) {
    if (!is_ready()) {
        return;
    }

    pwm_pin_set_usec(_pwm_devs[0], 4, _period, _get_pulse(outputs[0]), 0);
    pwm_pin_set_usec(_pwm_devs[0], 3, _period, _get_pulse(outputs[1]), 0);
    pwm_pin_set_usec(_pwm_devs[0], 2, _period, _get_pulse(outputs[2]), 0);
    pwm_pin_set_usec(_pwm_devs[1], 1, _period, _get_pulse(outputs[3]), 0);

#ifdef CONFIG_BOARD_STM32F3_DISCO
    // Put correct configuration into stm32f303 register..
    volatile int * volatile reg = (volatile int*)0x40012C20;
    *reg = 0x1444;
#endif
}

void PWM::stop_motors() {
    static constexpr float zeros[] = {0.0f, 0.0f, 0.0f, 0.0f};

    write(zeros);
}
