# Copyright (c) 2022 Jonathan Hahn
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_ZEPHLY_MIXER_LIB mixer.cpp)
