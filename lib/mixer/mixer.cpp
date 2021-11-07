#include "mixer.hpp"

static float B_inv[4][4] = {
    { 1.f,  1.f, -1.f, 1.f},
    { 1.f, -1.f,  1.f, 1.f},
    {-1.f, -1.f, -1.f, 1.f},
    {-1.f,  1.f,  1.f, 1.f}
};

void simple_mix(Command cmd, float (&outputs)[4]) {

    float inputs[4] = {cmd.roll, cmd.pitch, cmd.yaw, cmd.thrust};

    for (int i=0; i<4; i++) {
        // reset outputs
        outputs[i] = 0.f;

        for (int j=0; j<4; j++) {
            // calculate new mixed values
            outputs[i] += B_inv[i][j] * inputs[j];
        }

        // clamp output
        if (outputs[i] < 0.f) { outputs[i] = 0.f; }
        if (outputs[i] > 1.f) { outputs[i] = 1.f; }
    }
}
