#include "pwm.hpp"
#include <zephyr.h>
#include <drivers/pwm.h>

PWM::PWM(const struct device* pwm_dev) : _pwm_dev(pwm_dev) {};

bool PWM::is_ready() const {
    return device_is_ready(_pwm_dev);
}

void PWM::write(const float (&outputs)[4]) {
    if (!is_ready()) {
        return;
    }

    for (int i=0; i<4; i++) {
        uint32_t pulse = _min_duty + static_cast<uint32_t>( (_max_duty - _min_duty) * outputs[i]);
        pwm_pin_set_usec(_pwm_dev, i+1, _period, pulse, 0);
    }

#ifdef CONFIG_BOARD_STM32F3_DISCO
    // Put correct configuration into stm32f303 register..
    volatile int * volatile reg = (volatile int*)0x40012C20;
    *reg = 0x1444;
#endif
}
