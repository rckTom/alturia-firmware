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

#include "alturia.h"
#include "beeper.h"
#include <logging/log_ctrl.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(alturia, CONFIG_LOG_DEFAULT_LEVEL);

void __attribute__((noreturn)) panic(void)
{
	log_panic();
	k_panic();
	while(1);
}

void start_delay(int delay) {
	while (delay-- > 0) {
		if (delay >= 3) {
			k_sleep(K_SECONDS(1));
			beep(50,400);
		}
		else if (delay >= 2) {
			k_sleep(K_MSEC(500));
			beep(50,400);
			k_sleep(K_MSEC(500));
			beep(50,400);
		} else {
			k_sleep(K_MSEC(250));
			beep(50,400);
			k_sleep(K_MSEC(250));
			beep(50,450);
			k_sleep(K_MSEC(250));
			beep(50,400);
			k_sleep(K_MSEC(250));
			beep(50,450);
		}
	}
}

int next_datalog_path(char *path, size_t n)
{
	struct fs_dirent entry;
	char _path[32];
	int rc;

	while (true) {
		rc = fs_stat(_path, &entry);
		if(rc == -ENOENT){
			break;
		} else if (rc <= 0) {
			return rc;
		}
	}

	strlcpy(path, _path, n);
	return 0;
}

int check_free_space(void)
{
	struct fs_statvfs stats;
	u32_t free_space;
	int rc;

	rc = fs_statvfs(ALTURIA_FLASH_MP, &stats);

	free_space = stats.f_bfree * stats.f_bsize;

	LOG_INF("Free flash space: %d bytes", free_space);

	if (free_space <= ALTURIA_MINIMUM_FREE_SPACE) {
		return -1;
	}

	return 0;
}
