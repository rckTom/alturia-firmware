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
#include "rocket/signal_processing.h"
#include "rocket/flightstate.h"
#include "signals.h"
#include "events2.h"

LOG_MODULE_DECLARE(alturia);

extern struct event2 event_boot;

void main(void)
{
	int rc = init_fs();
	if (rc != 0) {
		LOG_ERR( "Unable to initialize file system");
		return;
	}

	daq_set_api_provider(sensor_daq_get_api_provider());
	daq_set_sample_interval(K_MSEC(10));
	daq_start();

	lua_engine_init();
	lua_engine_dofile("/lfs/user/test.lua");

	signal_processing_init();
	k_sleep(K_SECONDS(1));
	event2_fire(&event_boot);

	while(true) {
		// main event loop. This gets syncronized to daq frequency in signal_processing()
		signal_processing_main();
		flightstate_update();
		//logging();
	}
	//configure_pil();
}
