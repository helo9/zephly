# Copyright (c) 2022 Jonathan Hahn
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_FFC_SRXL2_RC_DRIVER 
    srxl2.c
    spm_srxl.c)
