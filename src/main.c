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
#include "sysinit.h"
#include <fs/fs.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_execution_engine.h"
#include "daq.h"
#include "datalogger.h"
#include "util.h"
#include "is_atmosphere.h"
#include "kalman_filter.h"
LOG_MODULE_DECLARE(alturia);

struct databuf  {
	int64_t t; 
	float pressure, ax, ay, az, gx, gy, gz;
} __packed;

void main(void)
{
	struct conf_desc *conf = NULL;
	int rc = init_fs();
	if (rc != 0) {
		LOG_ERR( "Unable to initialize file system");
		return;
	}
	lua_engine_init();
	lua_engine_dofile("/lfs/user/test.lua");
	configure_pil();
}
