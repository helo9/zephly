#include <zephyr.h>
#include <ztest.h>
#include <stdio.h>
#include <pid.h>
#include <lowpass.h>

#include "test_pid_data.inc"
#include "test_low_pass_data.inc"

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
                "wrong integrator value (row %d)", i);
        zassert_within(y, test_data[i][2], 0.001, 
                "wrong output value (row %d)", i);
    }
}

void test_lowpass() {
    float f_s = 1000.0f;
    float f_c = 50.0f;

    struct LowPass lp;

    lowpass_init(&lp, f_s, f_c);

    zassert_within(lp.alpha, 0.04761f, 0.001,
            "wrong parameter alpha set", NULL);

    for (int i=0; i<1000; i++) {
        float y = lowpass_update(&lp, test_data_lp[i][0]);

        zassert_within(y, test_data_lp[i][1], 0.001,
                "wrong output value (row %d)", i);
    }
}

void test_main(void) {
	ztest_test_suite(pid_tests,
		ztest_unit_test(test_simple_pid),
        ztest_unit_test(test_lowpass)
	);

	ztest_run_test_suite(pid_tests);
}
