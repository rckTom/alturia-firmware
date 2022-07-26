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

#include "util.h"
#include <zephyr/fs/fs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(alturia, 3);

bool file_exists(char *path)
{
	struct fs_dirent entry;

	return (fs_stat(path, &entry) == 0);
}

char* z_fgets(char* s, int n, struct fs_file_t *file)
{
    int c;
    int res = 0;
    char* cs;
    cs = s;

    while (--n > 0 && (res = fs_read(file, &c, 1) == 1))
    {
    // put the input char into the current pointer position, then increment it
    // if a newline entered, break
    if ((*cs++ = c) == '\n')
        break;
    }

    *cs = '\0';
    return (res <= 0 && cs == s) ? NULL : s;
}

float sensor_value_to_float(struct sensor_value *sval)
{
	return ((float) sval->val1 +
	       ((float) sval->val2) / 1000000.0f);
}

int get_log_count(const char* dir_path, int *min_log_num, int *max_log_num,
		  int* count)
{
	*min_log_num = INT_MAX;
	*max_log_num = 0;
	*count = 0;
	struct fs_dir_t dir = {0};
	int rc;

	rc = fs_opendir(&dir, dir_path);
	if (rc != 0) {
		LOG_ERR("can not open dir");
		return rc;
	}

	while (1) {
		struct fs_dirent entry;
		rc = fs_readdir(&dir, &entry);

		if (rc != 0) {
			LOG_ERR("error read dir");
			goto out;
		}

		if (entry.name[0] == 0) {
			break;
		}

		if (entry.type == FS_DIR_ENTRY_DIR) {
			continue;
		}

		const char *s = strstr(entry.name, "log");
		if (s == NULL || s != entry.name) {
			continue;
		}

		char *e = entry.name;
		int num = strtol(entry.name + 3, &e, 10);

		if (e == entry.name + 3) {
			continue;
		}

		(*count)++;

		if (num > *max_log_num) {
			*max_log_num = num;
		} else if (num < *min_log_num) {
			*min_log_num = num;
		}
	}

out:
	rc = fs_closedir(&dir);
	return rc;
}

int get_log_path(char *path, size_t n, int num)
{
	int rc = snprintf(path, n, "/lfs/data/log%d.dat", num);

	if (rc < 0 || rc > n) {
		return -ENOMEM;
	}

	return 0;
}
