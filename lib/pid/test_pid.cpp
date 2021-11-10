#include "pid.hpp"
#include "test_pid_data.inc"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <iostream>

using namespace Catch;

TEST_CASE("validate pid outputs", "[PID]") {
    float K = 5.0f;
    float P = 1.0f;
    float I = 0.01f;
    float D = 0.2f;
    float dt = 0.1f;

    PID pid(K, P, I, D);

    // initalize
    pid.update(0.0f, 0.0f, dt);

    for (auto elem : test_data) {
        float command = pid.update(elem[0], elem[1], dt);

        REQUIRE( pid.get_integral_value() == Approx(elem[3]) );
        REQUIRE( command == Approx(elem[2]).margin(0.001) );
    }
}
