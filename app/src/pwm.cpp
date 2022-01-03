#include "pwm.hpp"
#include <zephyr.h>
#include <drivers/pwm.h>
#include <devicetree.h>

#define NODE_OKAY(node) DT_NODE_HAS_STATUS(DT_NODELABEL(node), okay)
#define GET_PWM(node) DEVICE_DT_GET(DT_NODELABEL(node))
#define GET_PWM_NUM_PINS(node) 0

PWM::PWM() {
#if NODE_OKAY(pwm1)
    _pwm_devs[0] = GET_PWM(pwm1);
    _pwm_num_of_pins[0] = 4; //GET_PWM_NUM_PINS(pwm1);
#endif
#if NODE_OKAY(pwm2)
    _pwm_devs[1] = GET_PWM(pwm2);
    _pwm_num_of_pins[1] = GET_PWM_NUM_PINS(pwm2);
#endif
#if NODE_OKAY(pwm3)
    _pwm_devs[2] = GET_PWM(pwm3);
    _pwm_num_of_pins[2] = GET_PWM_NUM_PINS(pwm3);
#endif
#if NODE_OKAY(pwm4)
    _pwm_devs[3] = GET_PWM(pwm4);
    _pwm_num_of_pins[3] = GET_PWM_NUM_PINS(pwm4);
#endif
};

bool PWM::is_ready() const {
    bool ret = true;

#if NODE_OKAY(pwm1)
    ret &= device_is_ready(_pwm_devs[0]);
#endif
#if NODE_OKAY(pwm2)
    ret &= device_is_ready(_pwm_devs[1]);
#endif
#if NODE_OKAY(pwm3)
    ret &= device_is_ready(_pwm_devs[2]);
#endif
#if NODE_OKAY(pwm4)
    ret &= device_is_ready(_pwm_devs[3]);
#endif

    return ret;
}

void PWM::write(const float (&outputs)[4]) {
    if (!is_ready()) {
        return;
    }

    int dev_idx = 0;
    int last_dev_swap_idx = 0;

    for (int i=0; i<4; i++) {

        if (_pwm_devs[dev_idx] == NULL || (i - last_dev_swap_idx) >= _pwm_num_of_pins[dev_idx]) {
            dev_idx++;
        }

        if (dev_idx >= 4) {
            break;
        }

        uint32_t pulse = _min_duty + static_cast<uint32_t>( (_max_duty - _min_duty) * outputs[i]);
        int pin_num = i+1 - last_dev_swap_idx;
        pwm_pin_set_usec(_pwm_devs[dev_idx], pin_num, _period, pulse, 0);
    }

#ifdef CONFIG_BOARD_STM32F3_DISCO
    // Put correct configuration into stm32f303 register..
    volatile int * volatile reg = (volatile int*)0x40012C20;
    *reg = 0x1444;
#endif
}
