#pragma once

#include <msgs/msgs.h>

enum SATURATION_RESULT {
    NO_SATURATION_OCCURED,
    DESATURATION_PERFORMED
};

/**
 * @brief Simplest mixer form of a mixer, just uses clipping to optimize output values
 * 
 * @param cmd the [roll, pitch, yaw, throttle] demands, expected to be -1.0 to 1.0 except throttle 
 *          which should be 0.0 to 1.0
 * @param outputs the values modified by the function. holds values for each channel/motor with a range of 0.0 to 1.0 
 */

void simple_mix(const Command &cmd, float (&outputs)[4]) ;

/**
 * @brief Mix using a px4 airmode style algorithm
 * 
 * @param cmd the [roll, pitch, yaw, throttle] demands, expected to be -1.0 to 1.0 except throttle 
 *          which should be 0.0 to 1.0
 * @param outputs the values modified by the function. holds values for each channel/motor with a range of 0.0 to 1.0
 *
 * @return NO_SATURATION_OCCURED or DESATURATION_PERFORMED
 */
enum SATURATION_RESULT px_airmode_mix(const Command &cmd, float (&outputs)[4]);

/**
 * @brief Get the command which recreates outputs without saturation
 * 
 * @param cmd the input, which is adapted
 * @param outputs the output to recreate
 */
void mixer_get_desired_cmd(Command &cmd, const float (&outputs)[4]);
