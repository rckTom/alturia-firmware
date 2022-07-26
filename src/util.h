/*
 * Alturia Firmware - The firmware for the alturia flight computer
 *
 * Copyright (c) Thomas Schmid, 2019
 *
 * Authors:
 *  Thomas Schmid <tom@lfence.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 3.  See
 * the COPYING file in the top-level directory.
 */

#ifndef ALTURIA__UTIL__H
#define ALTURIA__UTIL__H

#include <zephyr/fs/fs.h>
#include "stdbool.h"
#include <zephyr/drivers/sensor.h>

bool file_exists(char *path);
char* z_fgets(char* s, int n, struct fs_file_t *file);
float sensor_value_to_float(struct sensor_value *val);
int get_log_count(const char* dir_path, int *min_log_num,
			 int *max_log_num, int* count);
int get_log_path(char *path, size_t n, int num);
int get_next_log_path(char *path, size_t n);

#endif
