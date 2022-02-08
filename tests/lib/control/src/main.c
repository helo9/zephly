#include <zephyr.h>
#include <ztest.h>
#include <stdio.h>
#include <pid.h>
#include <lowpass.h>
#include <ratecontrol.h>

#include "test_pid_data.inc"
#include "test_low_pass_data.inc"
#include "test_ratecontrol.inc"
#include "ztest_assert.h"

#define MAXIMAL_ALLOWED_DELTA 0.0001f

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
        zassert_within(y, test_data[i][2], MAXIMAL_ALLOWED_DELTA, 
                "wrong output value (row %d)", i);
    }
}

void test_lowpass() {
    float f_s = 1000.0f;
    float f_c = 50.0f;

    struct LowPass lp;

    lowpass_init(&lp, f_s, f_c);

    zassert_within(lp.alpha, 0.04761f, MAXIMAL_ALLOWED_DELTA,
            "wrong parameter alpha set", NULL);

    for (int i=0; i<1000; i++) {
        float y = lowpass_update(&lp, test_data_lp[i][0]);

        zassert_within(y, test_data_lp[i][1], MAXIMAL_ALLOWED_DELTA,
                "wrong output value (row %d)", i);
    }

    lowpass_reset(&lp);
}

void test_ratecontrol_pi() {
    struct RateControl ratecontrol;

    ratecontrol_init(&ratecontrol, "roll");

    /* Input first value to have initial values everywhere */
    ratecontrol_update(&ratecontrol, 0.0f, 0.0f);

    /* Raise reference and check reaction */
    ratecontrol_update(&ratecontrol, 1.0f, 0.0f);
    
    /* Hold reference another time, to get integrator reaction */
    float out = ratecontrol_update(&ratecontrol, 1.0f, 0.0f);

    zassert_true(out > 0.0f, "Incorrect output");
    zassert_true(ratecontrol.i_val > 0.0f, "Incorrect integrator value.");
    zassert_true(ratecontrol.p_val > 0.0f, "Incorrect proportional value.");
    /* No change in measurement -> no d value to be exected */
    zassert_within(ratecontrol.d_val, 0.0f, MAXIMAL_ALLOWED_DELTA, "Incorrect differential value.");

    /* Lower reference and check reaction */
    ratecontrol_update(&ratecontrol, -1.0f, 0.0f);
    /* Repeat, integrator should be at 0 afterwards */
    ratecontrol_update(&ratecontrol, -1.0f, 0.0f);
    
    /* Hold reference two more times, to get integrator reaction */
    ratecontrol_update(&ratecontrol, -1.0f, 0.0f);
    out = ratecontrol_update(&ratecontrol, -1.0f, 0.0f);

    zassert_true(out < 0.0f, "Incorrect output");
    zassert_true(ratecontrol.i_val < 0.0f, "Incorrect integrator value.");
    zassert_true(ratecontrol.p_val < 0.0f, "Incorrect proportional value.");
    /* No change in measurement -> no d value to be exected */
    zassert_within(ratecontrol.d_val, 0.0f, MAXIMAL_ALLOWED_DELTA, "Incorrect differential value.");
}

void test_ratecontrol_d() {
    float out;
    struct RateControl ratecontrol;

    ratecontrol_init(&ratecontrol, "roll");

    /* Input first value to have initial values everywhere */
    ratecontrol_update(&ratecontrol, 0.0f, 0.0f);

    /* Create Step in measurement */
    out = ratecontrol_update(&ratecontrol, 0.0f, 1.0f);

    zassert_true(ratecontrol.d_val < 0, "Incorrect sign for differential fraction.");
}

void test_ratecontrol_saturation_states() {
    struct RateControl ratecontrol;

    ratecontrol_init(&ratecontrol, "roll");

    /* Input first value to have initial values everywhere */
    ratecontrol_update(&ratecontrol, 0.0f, 0.0f);

    ratecontrol_set_saturation(&ratecontrol, POSITIVE_SATURATION);

    /* Create step in reference signal */
    ratecontrol_update(&ratecontrol, 1.0f, 0.0f);

    zassert_within(ratecontrol.i_sum, 0.0, MAXIMAL_ALLOWED_DELTA, "Integral value increased, although saturized.");
}

void test_ratecontrol_backcalculation() {
    const float out_des = 2.0f;
    float out;
    struct RateControl ratecontrol;

    ratecontrol_init(&ratecontrol, "roll");

    /* Input first value to have initial values everywhere */
    ratecontrol_update(&ratecontrol, 0.0f, 0.0f);

    out = ratecontrol_update(&ratecontrol, 1.0f, 0.0f);

    /* Assert there is something to desaturate */
    zassert_true(out > out_des, "No desaturation necessary, adapt `out_des`");

    ratecontrol_backcalculation(&ratecontrol, out_des);

    out = ratecontrol_update(&ratecontrol, 1.0f, 0.0f);

    zassert_within(out, out_des, MAXIMAL_ALLOWED_DELTA, "Backcalculation did not work");
}

void test_ratecontrol_reset() {
    struct RateControl ratecontrol;

    ratecontrol_init(&ratecontrol, "roll");

    /* Set non 0 values, to check reset */
    ratecontrol.last_y = 1.0f;
    ratecontrol.d_ready = true;

    ratecontrol.i_sum = 1.0f;

    ratecontrol_reset(&ratecontrol);

    bool reset_check = (ratecontrol.last_y == 0.0f)
            && (!ratecontrol.d_ready)
            && (ratecontrol.i_sum == 0.0f);

    zassert_true(reset_check, "Not properly reset");
}

void test_main(void) {
	ztest_test_suite(pid_tests,
		ztest_unit_test(test_simple_pid),
        ztest_unit_test(test_lowpass),
        ztest_unit_test(test_ratecontrol_pi),
        ztest_unit_test(test_ratecontrol_d),
        ztest_unit_test(test_ratecontrol_saturation_states),
        ztest_unit_test(test_ratecontrol_backcalculation),
        ztest_unit_test(test_ratecontrol_reset)
	);

	ztest_run_test_suite(pid_tests);
}
