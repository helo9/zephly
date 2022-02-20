#include <stdint.h>

#ifndef SAMPLE_MSG_H
#define SAMPLE_MSG_H

union  measurement_msg {
        struct __attribute__((__packed__)) {
            uint16_t ts : 16;
            int32_t gyro_x : 20;
            int32_t gyro_y : 20;
            int32_t gyro_z : 20;
        } values;
        uint8_t raw[10];
};

#endif /* SAMPLE_MSG_H */
