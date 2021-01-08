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
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_execution_engine.h"
#include "daq.h"
#include "datalogger.h"
#include "util.h"
LOG_MODULE_DECLARE(alturia);

struct databuf  {
	unsigned long t; 
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


	char path[64];
	int log_min, log_max, next_log;
	int log_count = get_log_count("/lfs/data", &log_min, &log_max, &next_log);
	get_log_path(path, 64, next_log + 1);
	dl_open_log(path);
	dl_add_track_format_chunk(0 , "qfffffff");
	dl_add_track_names_chunk(0, "time, pressure, ax, ay, az, gx, gy, gz");
	daq_start();
	int64_t start = k_uptime_get();
	while((k_uptime_get() - start) <= 10000) {
		daq_sync();
		struct log_data *data;
		struct databuf data_storage;
		dl_alloc_track_data_buffer(&data, 0, sizeof(data_storage));
		data_storage.t = k_uptime_get();
		data_storage.pressure = sensor_value_to_float(&press_sample);
		data_storage.ax = sensor_value_to_float(acc_sample);
		data_storage.ay = sensor_value_to_float(acc_sample + 1);
		data_storage.az = sensor_value_to_float(acc_sample + 2);
		data_storage.gx = sensor_value_to_float(gyr_sample);
		data_storage.gy = sensor_value_to_float(gyr_sample + 1);
		data_storage.gz = sensor_value_to_float(gyr_sample + 2);
		dl_add_track_data(data);
	}

	dl_close_log();

	while(1) {
		k_sleep(K_MSEC(1000));
	}
}
