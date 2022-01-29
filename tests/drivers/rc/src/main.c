#include <zephyr.h>
#include <ztest.h>
#include <stdio.h>

#include "test_rate_setpoint.inc"

float apply_superexpo(float val, float e, float g);

void test_superexpo() {
    for (int i=0; i<240; i++) {
        const float *row = test_data_setpoint[i];
        const float out = apply_superexpo(row[0], row[1], row[2]);
        
        zassert_within(out, row[3], 0.001,
                "wrong output value (row %d)", i);
    }
}

void test_main(void) {
    ztest_test_suite(rc_tests,
        ztest_unit_test(test_superexpo)
    );

    ztest_run_test_suite(rc_tests);
}
