#pragma once

#include <msgs/msgs.h>

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
 */
void px_airmode_mix(const Command &cmd, float (&outputs)[4]);
