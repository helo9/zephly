#include <math.h>
#include "mixer.hpp"

#define FLT_EPSILON __FLT_EPSILON__

/* the motor matrix and inverse in transposed form */
static constexpr float motor_matrix_transposed[4][4] = {
	{ 0.250f,  0.250f, -0.250f,  0.250f},
	{ 0.250f, -0.250f,  0.250f,  0.250f},
	{-0.250f, -0.250f, -0.250f,  0.250f},
	{-0.250f,  0.250f,  0.250f,  0.250f}
};

static constexpr float motor_matrix_inverse_transposed[4][4] = {
	{ 1.000f,  1.000f, -1.000f, -1.000f},
	{ 1.000f, -1.000f, -1.000f,  1.000f},
	{-1.000f,  1.000f, -1.000f,  1.000f},
	{ 1.000f,  1.000f,  1.000f,  1.000f}
};

/* yaw and thrust multipliers used for desaturation in airmode mixer */
static constexpr const float (&yaw_scales)[4] = motor_matrix_inverse_transposed[2];
static constexpr const float (&thrust_scales)[4] = motor_matrix_inverse_transposed[3];

/**
 * @brief Get the value of the motor matrix inverse with none transposed indices
 * 
 * @param i row of motor matrix inverse
 * @param j column of motor matrix inverse
 * @return constexpr float the value at the given indices
 */
static constexpr float get_motor_matrix_inv_val(int i, int j) {
    return motor_matrix_inverse_transposed[j][i];
}

/**
 * @brief Get the value of the motor matrix with none transposed indices
 * 
 * @param i row of motor matrix inverse
 * @param j column of motor matrix inverse
 * @return constexpr float the value at the given indices
 */
static constexpr float get_motor_matrix_val(int i, int j) {
    return motor_matrix_transposed[j][i];
}

/**
 * @brief Translate the demands into output channel / motor commands
 * 
 * @param cmd the [roll, pitch, yaw, throttle] demands, expected to be -1.0 to 1.0 except throttle 
 *          which should be 0.0 to 1.0 
 * @param outputs the values modified by the function. holds values for each channel/motor with a range of 0.0 to 1.0
 */
static void mix(const Command &cmd, float (&outputs)[4]) {
    float inputs[4] = {cmd.roll, cmd.pitch, cmd.yaw, cmd.thrust};

    for (int i=0; i<4; i++) {
        /* reset outputs */
        outputs[i] = 0.f;

        for (int j=0; j<4; j++) {
            /* calculate new mixed values */
            outputs[i] += get_motor_matrix_inv_val(i, j) * inputs[j];
        }
    }
}

/**
 * @brief Limit the output vector using the given limits
 * 
 * @param outputs the vector to adapt
 * @param min_out lower limit
 * @param max_out upper limit
 *
 * @return true when output was clipped
 */
static bool clip(float (&outputs)[4], float min_out=0.f, float max_out=1.f) {
    bool clipping_needed = false;
    for (int i=0; i<4; i++) {
        if (outputs[i] < min_out) {
             outputs[i] = min_out;
             clipping_needed = true;
        }

        if (outputs[i] > max_out) {
            outputs[i] = max_out;
            clipping_needed = true;
        }
    }
    
    return clipping_needed;
}

/**
 * @brief Calculate the gain for des_vec to reduce desaturation of outputs vector the most
 * 
 * @param des_vec the vector which may be added to outputs multiplied by the gain to reduce saturation
 * @param outputs the actual output vector, potentially saturizes
 * @param min_out lower limit, a value in outputs below is saturized
 * @param max_out upper limit, a value in outputs above is saturized
 * @return float the gain for des_vec
 */
static float compute_desaturation_gain(const float (&des_vec)[4], float (&outputs)[4], float min_out=0.f, float max_out=1.f) {
    float k_min=0.0f, k_max=0.0f;

    for (int i=0; i<4; i++) {
        if (fabs(des_vec[i]) < FLT_EPSILON) {
            continue;
        }

        if (outputs[i] < min_out) {
            float k_tmp = (min_out - outputs[i]) / des_vec[i];

            if (k_tmp < k_min) { k_min = k_tmp; }
            if (k_tmp > k_max) { k_max = k_tmp; }
        }

        if (outputs[i] > max_out) {
            float k_tmp = (max_out - outputs[i]) / des_vec[i];

            if (k_tmp < k_min) { k_min = k_tmp; }
            if (k_tmp > k_max) { k_max = k_tmp; }
        }
    }

    return k_min + k_max;
}

/**
 * @brief Use des_vec to reduce saturation of outputs as much as possible
 * 
 * @param des_vec the desaturation vector which may be used multiplied by any gain to reduce saturation
 * @param outputs the output values to modify
 * @param min_out lower limit, a value in outputs below is saturized
 * @param max_out upper limit, a value in outputs above is saturized
 * 
 * @return true when desaturation was needed
 */
static bool desaturate(const float (&des_vec)[4], float (&outputs)[4], float min_out=0.f, float max_out=1.f) {
    float k1 = compute_desaturation_gain(des_vec, outputs, min_out, max_out);

    for (int i=0; i<4; i++) {
        outputs[i] += k1 * des_vec[i];
    }

    float k2 = compute_desaturation_gain(des_vec, outputs, min_out, max_out);

    for (int i=0; i<4; i++) {
        outputs[i] += 0.5 * k2 * des_vec[i];
    }

    if (k1 == 0.0f && k2 == 0.0f) {
        return true;
    } else {
        return false;
    }
}

void simple_mix(const Command &cmd, float (&outputs)[4]) {
    /* do the actual mixing */
    mix(cmd, outputs);

    /* clip too big or too small values */
    clip(outputs);
}

enum SATURATION_RESULT px_airmode_mix(const Command &cmd, float (&outputs)[4]) {

    bool was_saturated = false;

    mix(cmd, outputs);

    was_saturated |= desaturate(thrust_scales, outputs);

    was_saturated |= desaturate(yaw_scales, outputs);

    was_saturated |= clip(outputs);

    if (was_saturated) {
        return DESATURATION_PERFORMED;
    } else {
        return NO_SATURATION_OCCURED;
    }
}

void mixer_get_desired_cmd(Command &cmd, const float (&outputs)[4]) {
    float desired_inputs[4] = {};

    for (int i=0; i<4; i++) {

        for (int j=0; j<4; j++) {
            /* calculate new mixed values */
            desired_inputs[i] += get_motor_matrix_val(i, j) * outputs[j];
        }
    }

    cmd.roll = desired_inputs[0];
    cmd.pitch = desired_inputs[1];
    cmd.yaw = desired_inputs[2];
    cmd.thrust = desired_inputs[3];
}
