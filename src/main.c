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

#define _DEFAULT_SOURCE 1
#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>
#include <usb/usb_device.h>
#include "execution_engine.h"
#include "sysinit.h"
#include <fs/fs.h>

LOG_MODULE_DECLARE(alturia);

void main(void)
{
	struct conf_desc *conf = NULL;
	int rc = init_fs();
	if (rc != 0) {
		LOG_ERR( "Unable to initialize file system");
		return;
	}

	//read execution engine context
	struct fs_dirent dirstat;
	rc = fs_stat("/lfs/config/config.bin", &dirstat);
	if (rc!= 0) {
		LOG_INF("unable to stat file");
		goto error;
	}
	
	struct fs_file_t fid;
	rc = fs_open(&fid, "/lfs/config/config.bin", FS_O_READ);
	if (rc != 0) {
		LOG_ERR("unable to open file");
		goto error;
	}

	conf = k_malloc(dirstat.size);
	if (conf == NULL) {
		LOG_ERR("unable to allocate enough memory for config");
		goto error;
	}

	LOG_INF("file size: %d", dirstat.size);
	rc = fs_read(&fid, conf, dirstat.size);
	event_loop(conf);

error:
	k_free(conf);
	while(1) {
		k_sleep(K_MSEC(1000));
	}
}
