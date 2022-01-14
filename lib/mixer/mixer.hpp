#pragma once

#include <msgs/msgs.h>

/**
 * @brief Mix based on fixed motor matrix
 * 
 * @param cmd inputs
 * @param outputs 
 */
void simple_mix(Command cmd, float (&outputs)[4]);
