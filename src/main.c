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
#include "execution_engine.h"
#include "led.h"
#include "sysinit.h"

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
	
	struct color_hsv white = {
		.h = 120,
		.s = 1.0,
		.v = 1.0
	};
	led_fade_to_hsv(&white, 0.3);

	while(1) {
		k_sleep(K_SECONDS(10));
	}
}
