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
#include "sysinit.h"
#include "alturia.h"
#include "beeper.h"
#include "configuration.h"
#include "datalogger.h"
#include "arm_math.h"

LOG_MODULE_DECLARE(alturia);

void main(void)
{
	int rc;
	char flight_cfg_path[32] = ALTURIA_FLASH_MP;

	rc = init_peripherals();
	if (rc != 0) {
		LOG_ERR("Unable to initialize gpios");
		panic();
	}

	rc = init_fs();
	if (rc != 0) {
		LOG_ERR("Unable to initialize filesystem");
		panic();
	}

	rc = read_sys_config(ALTURIA_FLASH_MP"/config/syscfg.ini");
	if (rc != 0) {
		LOG_ERR("Can not read systemconfig. Aborting");
		panic();
	}

	beeper_set_volume(get_sys_config()->beeper_volume);
	strlcat(flight_cfg_path, get_sys_config()->flightcfg_path,
		ARRAY_SIZE(flight_cfg_path));

	LOG_DBG("%s\n", log_strdup(flight_cfg_path));
	rc = read_flight_config(flight_cfg_path);
	if (rc != 0) {
		LOG_ERR("Unable to read flightcfg. Aborting");
		return;
	}

	while(1) {
		k_sleep(1000);
	}
}
