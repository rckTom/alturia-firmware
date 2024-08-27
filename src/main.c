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
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <stdio.h>
#include <zephyr/usb/usb_device.h>
#include "sysinit.h"
#include <zephyr/fs/fs.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_execution_engine.h"
#include "daq.h"
#include "rocket/signal_processing.h"
#include "rocket/flightstate.h"
#include "rocket/data_logging.h"
#include "signals.h"
#include "events2.h"
#include "alturia.h"
#include "util.h"
#include "datalogger.h"
#include "beeper.h"
#include "rocket/beeper_logic.h"
#include "config_yaml.h"

LOG_MODULE_DECLARE(alturia);

DECLARE_SIGNAL(signal_h);
DECLARE_SIGNAL(signal_h_raw);
DECLARE_SIGNAL(signal_v_w);
DECLARE_SIGNAL(signal_pressure);
DECLARE_SIGNAL(signal_a_raw);
DECLARE_SIGNAL(signal_omega_raw);
DECLARE_SIGNAL(signal_q);

extern struct event2 event_boot;

void evt_callback(struct event2 *evt) {
	LOG_INF("event %s", evt->evt_name);
}

int main(void)
{
	int rc = 0;
	init_config(NULL);

	daq_set_api_provider(sensor_daq_get_api_provider());
	daq_set_sample_interval(K_MSEC(10));
	daq_start();

	lua_engine_init();
	#if CONFIG_FILE_SYSTEM
	lua_engine_dofile("/lfs/config/flightconfig.lua");
	#endif

	STRUCT_SECTION_FOREACH(event2, evt) {
		event2_register_callback(evt, evt_callback);
	}

	signal_processing_init();
	flightstate_init();
	rocket_beeper_logic_start();

	k_sleep(K_SECONDS(1));
	event2_fire(&event_boot);

	#if CONFIG_FILE_SYSTEM
	int minlog, maxlog, count;
	rc = get_log_count("/lfs/data", &minlog, &maxlog, &count);
	char log_file_path[64];
	get_log_path(log_file_path, 64, maxlog + 1);
	rocket_data_logging_set_name(log_file_path);
	#endif

	mission_state_t last_state = STATE_STARTUP;
	while(true) {
		// main event loop. This gets syncronized to daq frequency in signal_processing()
		signal_processing_main();
		flightstate_update();

		if (flightstate_get_state() != last_state) {
			last_state = flightstate_get_state();
			LOG_INF("changed flightstate to %d", last_state);
		}

		#if CONFIG_FILE_SYSTEM
		rocket_data_logging();
		#endif
	}
	
	while(true) {
		k_sleep(K_MSEC(1000));
	}

	return 0;
}
