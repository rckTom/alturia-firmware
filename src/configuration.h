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

#ifndef __ALTURIA_CONFIGURATION_H__
#define __ALTURIA_CONFIGURATION_H__

#include <stdbool.h>

#define SYS_CONFIG_SCHEMA \
	/*    TYPE, NAME		, EXT , SECTION */ 	\
	FIELD(char, flightcfg_path	, [64], 	) 	\
	FIELD(char, owner		, [32], 	) 	\
	FIELD(int , beeper_volume	,     ,		)

#define FLIGHT_CONFIG_SCHEMA \
	FIELD(int, start_delay		,     ,		)

struct sys_config {
	#define FIELD(type, name, name_ext, section) type name name_ext;
		SYS_CONFIG_SCHEMA
	#undef FIELD
};

struct sys_config_valid {
	#define FIELD(type, name, name_ext, section) \
		      uint8_t name : 1;
		SYS_CONFIG_SCHEMA
	#undef FIELD
	bool config_valid;
};

struct flight_config {
	#define FIELD(type, name, name_ext, section) type name name_ext;
		FLIGHT_CONFIG_SCHEMA
	#undef FIELD
};

struct flight_config_valid {
	#define FIELD(type, name, name_ext, section) \
		      uint8_t name : 1;
		FLIGHT_CONFIG_SCHEMA
	#undef FIELD
	bool config_valid;
};

int read_sys_config(const char* path);
int read_flight_config(const char* path);

const struct sys_config *get_sys_config();
const struct flight_config *get_flight_config();

#endif /* __ALTURIA_CONFIGURATION_H__ */
