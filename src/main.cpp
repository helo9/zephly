#include "sensors.hpp"
#include "kernel.h"
#include <device.h>
#include <stdio.h>
#include <zephyr.h>
#include <math.h>

#include <attitude_estimation.hpp>
#include "pwm.hpp"
#include "rc/rc.hpp"

constexpr double DEG2RAD = 0.017453292519943295;

using namespace AttitudeEstimation;

float outputs[] = {0.5f, 0.5f, 0.5f, 0.5f};
RCInput rc_input;

void report_state(const Quaternion<double> &q, const Vector<double, 3> &w, const Vector<double, 3> &a);
void cmd2outputs(RCInput cmd, float *outputs);
void rc_print(RCInput rc_input);

int main() {
	auto &pwm = PWM::init();

	if(rc_init() < 0) {
		printk("RC Initialization failed!\n");
		return -1;
	}

	if (!initialize_sensors() || !pwm.is_ready()) {
		printk("Initialization failed!");
		return -1;
	}

	pwm.write(outputs);

	/* initial attitude */
	Quaternion<double> q = {1.0, 0.0, 0.0, 0.0};

	rc_run();
	
	while (true) {

		rc_input = rc_get();
		rc_print(rc_input);

		cmd2outputs(rc_input, outputs);

		pwm.write(outputs);
		printf("o:\t%f\t%f\t%f\t%f\n", outputs[0], outputs[1], outputs[2], outputs[3]);

		if (!update_measurements()) {
			/* Something went wrong, try again */
			printk("Update failed!");
			continue;
		}

		propagate_attitude<double>(q, get_rotation_speed(), get_delta_t());

		report_state(q, get_rotation_speed(), get_accelerations());

		k_sleep(K_MSEC(500));
	}

	return 0;
}

void report_state(const Quaternion<double> &q, const Vector<double, 3> &w, const Vector<double, 3> &a) {
	printf("%f %f %f %f", q.w, q.x, q.y, q.z);
	printf(" %u %f %f %f", k_uptime_get_32(), w.data[0], w.data[1], w.data[2]);
	printf(" %f %f %f \n", a.data[0], a.data[1], a.data[2]);
}

void rc_print(RCInput rc_input) {
	printk("rc:\t%d\t%d\t%d\t%d\n", rc_input.pitch, rc_input.roll, rc_input.throttle, rc_input.yaw);
}

inline float rc2out(uint16_t rc) {
	
	float tmp = (rc - 0x8000) / 43700.0f + 0.5f;

	if (tmp>1.0f) {
		return 1.0f;
	} else if (tmp <0.0f) {
		return 0.0f;
	} else {
		return tmp;
	}
}

void cmd2outputs(RCInput cmd, float *outputs) {
	outputs[0] = rc2out(cmd.pitch);
	outputs[1] = rc2out(cmd.roll);
	outputs[2] = rc2out(cmd.throttle);
	outputs[3] = rc2out(cmd.yaw);
}
