#include <zephyr.h>
#include <ztest.h>
#include <stdio.h>
#include <pid.h>

#include "test_pid_data.inc"

void test_simple_pid() {
    float k = 5.0f;
    float p = 1.0f;
    float i = 0.01f;
    float d = 0.2f;
    float dt = 0.1f;
    float ilim = 10.0f;

    struct PID pid;

    pid_init(&pid, k, p, i, d, dt, ilim);

    for (int i=0; i<3000; i++) {
        float y = pid_update(&pid, test_data[i][0], test_data[i][1]);
        zassert_within(pid.ival, test_data[i][3], 0.001, 
                "wrong integrator value (line %d)", i);
        zassert_within(y, test_data[i][2], 0.001, 
                "wrong output value (line %d)", i);
    }
}

void test_main(void) {
	ztest_test_suite(pid_tests,
		ztest_unit_test(test_simple_pid)
	);

	ztest_run_test_suite(pid_tests);
}
