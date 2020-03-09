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
#include <logging/log_ctrl.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(alturia, CONFIG_LOG_DEFAULT_LEVEL);

void __attribute__((noreturn)) panic(const char *str)
{
	log_panic();
	printk("%s", str);
	k_panic();
	while(1);
}

}
