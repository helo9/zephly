# Copyright (c) 2022 Jonathan Hahn
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_ICM20602_DRIVER icm20602.c)
zephyr_library_sources_ifdef(CONFIG_ICM20602_TRIGGER icm20602_trigger.c)
