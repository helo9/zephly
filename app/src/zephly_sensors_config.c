/*
 * Copyright (c) 2022 Jonathan Hahn
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <shell/shell.h>

#include <stdio.h>
#include <stdlib.h>

#include "zephly_sensors.h"

#if IS_ENABLED(CONFIG_SHELL)

static int zephly_cmd_set_gyro_offsets(const struct shell *sh, size_t argc, char **argv) {
    const float offset_x = atof(argv[1]);
    const float offset_y = atof(argv[2]);
    const float offset_z = atof(argv[3]);

    printf("set gyro offset to %f %f %f\n", offset_x, offset_y, offset_z);

    zephly_gyro_set_offsets(offset_x, offset_y, offset_z);

    return 0;
}

SHELL_CMD_ARG_REGISTER(set_gyro_offset, NULL, "Modify Gyro Offsets", zephly_cmd_set_gyro_offsets, 4, 0);

#endif /* CONFIG_SHELL */
