#pragma once

struct Command {
    float roll;
    float pitch;
    float yaw;
    float thrust;
};

void simple_mix(Command cmd, float (&outputs)[4]);
