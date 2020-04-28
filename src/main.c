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
#include "sysinit.h"
#include "alturia.h"
#include "beeper.h"
#include "configuration.h"
#include "datalogger.h"
#include "arm_math.h"
#include "is_atmosphere.h"
#include "daq.h"
#include "util.h"
#include "kalman_filter.h"
#include "drivers/uart.h"
#include "transformations_impl.h"
#include "math_ex.h"
#include "drivers/pwm.h"
#include "servos.h"
#include "events.h"
#include "actions.h"


LOG_MODULE_DECLARE(alturia);

void main(void)
{

	int rc;
	char flight_cfg_path[32] = ALTURIA_FLASH_MP;

	struct uart_config conf;
	conf.baudrate = 1000000;
	conf.data_bits = 8;
	conf.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;
	conf.parity = UART_CFG_PARITY_NONE;
	conf.stop_bits = UART_CFG_STOP_BITS_1;

	beeper_set_volume(10);
	rc = init_peripherals();
	if (rc != 0) {
		LOG_ERR("Unable to initialize gpios");
		k_oops();
	}

	rc = init_fs();
	if (rc != 0) {
		LOG_ERR("Unable to initialize filesystem");
		k_oops();
	}

/* 	rc = read_sys_config(ALTURIA_FLASH_MP"/config/syscfg.ini");
	if (rc != 0) {
		LOG_ERR("Can not read systemconfig. Aborting");
		goto idle;
	}

	beeper_set_volume(get_sys_config()->beeper_volume);
	strlcat(flight_cfg_path, get_sys_config()->flightcfg_path,
		ARRAY_SIZE(flight_cfg_path));

	LOG_DBG("%s\n", log_strdup(flight_cfg_path));
	rc = read_flight_config(flight_cfg_path);
	if (rc != 0) {
		LOG_ERR("Unable to read flightcfg. Aborting");
	}

idle: */
	struct beep_action_data beep_data = {
		.duration = 100,
		.pitch = 400,
	};

	struct event_timer_data timer_action_data = {
		.count = 2000,
		.timer_number = 0,
		.cyclic = true,
	};

	struct event_timer_data timer1_action_data = {
		.count = 2000,
		.timer_number = 1,
		.cyclic = false,
	};

	struct servo_action_data servo_data = {
		.servo_number = 0,
		.position = 1100,
	};

	struct servo_action_data reset_servo_data = {
		.servo_number = 0,
		.position = 2000,
	};

	struct action_beeper_volume_data beeper_volume = {
		.volume = 50,
	};

	struct action beep_action;
	struct action timer_action;
	struct action timer1_start_action;
	struct action timer_stop_action;
	struct action servo_move;
	struct action reset_servo;
	struct action change_volume;

	action_init(&beep_action, ACTION_BEEP, &beep_data);
	action_init(&timer_action, ACTION_TIMER_START, &timer_action_data);
	action_init(&timer1_start_action, ACTION_TIMER_START, &timer1_action_data);
	action_init(&timer_stop_action, ACTION_TIMER_STOP, &timer_action_data);
	action_init(&servo_move, ACTION_SERVO_SET, &servo_data);
	action_init(&reset_servo, ACTION_SERVO_SET, &reset_servo_data);
	action_init(&change_volume, ACTION_BEEPER_SET_VOLUME, &beeper_volume);

	numeric_t var1;
	var1.type = NUMERIC_TYPE_LONG;
	var1.asLong = 20000;

	numeric_t var2;
	var2.type = NUMERIC_TYPE_LONG;
	var2.asLong = 10000;

	condition_t cond;
	condition_t cond2;

	condition_init(&cond, COND_GOES_ABOVE, FS_VAR_SYSTEM_TIME, var1);
	condition_init(&cond2, COND_GOES_ABOVE, FS_VAR_SYSTEM_TIME, var2);

	timer_event_data_t timer_evt_param = {
		.timer_number = 0
	};

	timer_event_data_t timer1_evt_param = {
		.timer_number = 1
	};

	struct event boot_evt;
	struct event timer_evt;
	struct event timer1_evt;
	struct event tick_evt;
	struct event tick2_evt;

	event_initialize(&boot_evt, EVT_BOOT, NULL, NULL);
	event_initialize(&timer_evt, EVT_TIMER_EXPR, &timer_evt_param, NULL);
	event_initialize(&tick_evt, EVT_TICK, NULL, &cond);
	event_initialize(&timer1_evt, EVT_TIMER_EXPR, &timer1_evt_param, NULL);
	event_initialize(&tick2_evt, EVT_TICK, NULL, &cond2);

	event_action_append(&tick_evt, &timer_stop_action);
	event_action_append(&tick_evt, &servo_move);
	event_action_append(&tick_evt, &timer1_start_action);
	event_action_append(&boot_evt, &timer_action);
	event_action_append(&timer_evt, &beep_action);
	event_action_append(&timer1_evt, &reset_servo);
	event_action_append(&tick2_evt, &change_volume);

	event_append(&boot_evt);
	event_append(&timer_evt);
	event_append(&timer1_evt);
	event_append(&tick_evt);
	event_append(&tick2_evt);

	event_loop();
}
