#include <zephyr.h>
#include <ztest.h>
#include <stdio.h>
#include <pid.h>
#include <lowpass.h>

#include <mixer/mixer.hpp>

#include "test_data.inc"
#include "test_data_px_airmode.inc"

#define USE_ONLY_COMMAND(var) {var[0], var[1], var[2], var[3]}

void test_simple_mixer() {
	float outputs[4] = {};

	for (int i=0; i<25; i++) {
		Command cmd = USE_ONLY_COMMAND(test_data[i]);

		simple_mix(cmd, outputs);

		for (int j=0; j<4; j++) {
			
            zassert_within(outputs[j], test_data[i][4+j], 0.001,
					"wrong output value (row %d, col %d)", i, j);
        }
	}
}

void test_airmode_mixer() {
	float outputs[4] = {};

	for (int i=0; i<50; i++) {
		const Command cmd = USE_ONLY_COMMAND(test_data_airmode[i]);

		Command cmd_des;

		px_airmode_mix(cmd, outputs);

		/* check back calcuation support on the way */
		mixer_get_desired_cmd(cmd_des, outputs);
		float cmd_des_arr[] = {cmd_des.roll, cmd_des.pitch, cmd_des.yaw, cmd_des.thrust};

		for (int j=0; j<4; j++) {
            zassert_within(outputs[j], test_data_airmode[i][4+j], 0.01,
					"wrong output value (row %d, col %d)", i, j);

			zassert_within(cmd_des_arr[j], test_data_airmode[i][8+j], 0.01,
					"wrong desaturated value (row %d, col %d)", i, j);
        }
	}
}

void test_main(void) {
	ztest_test_suite(pid_tests,
		ztest_unit_test(test_simple_mixer),
		ztest_unit_test(test_airmode_mixer)
	);

	ztest_run_test_suite(pid_tests);
}
