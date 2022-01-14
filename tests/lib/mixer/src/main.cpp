#include <zephyr.h>
#include <ztest.h>
#include <stdio.h>
#include <pid.h>
#include <lowpass.h>

#include <mixer/mixer.hpp>

#include "test_data.inc"

#define USE_ONLY_COMMAND(var) {var[0], var[1], var[2], var[3]}

void test_mixer() {
	float outputs[4] = {};

	for (int i=0; i<25; i++) {
		Command cmd = USE_ONLY_COMMAND(test_data[i]);

		simple_mix(cmd, outputs);

		for (int j=0; j<4; j++) {
			
            zassert_within(outputs[j], test_data[i][4+j], 0.001,
					"wrong output value (row %d)", i);
        }
	}
}

void test_main(void) {
	ztest_test_suite(pid_tests,
		ztest_unit_test(test_mixer)
	);

	ztest_run_test_suite(pid_tests);
}
