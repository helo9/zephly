#include "stdio.h"
#include "ahrs.h"

int main() {

    float qw = 1.0f, qx = 0.0f, qy = 0.0f, qz = 0.0f;
    float wx = 0.1f, wy = 0.0f, wz = 0.0f;
    float dt = 0.1f;

    propagate_attitudef(&qw, &qx, &qy, &qz, wx, wy, wz, dt);

    printf("%f %f %f %f \n", qw, qx, qy, qz);

    return 0;
}
