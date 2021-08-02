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

#ifndef ALTURIA__MAIN__H
#define ALTURIA__MAIN__H

#include <zephyr.h>
#include <drivers/gpio.h>
#include <device.h>

#define ALTURIA_FLASH_MP                    "/lfs"
#define ALTURIA_MINIMUM_FREE_SPACE	    1024*3

#define KALMAN_MAX_ORDER		    CONFIG_KALMAN_MAX_ORDER

void __attribute__((noreturn)) panic(void);
void start_delay(int delay);
int next_datalog_path(char *path, size_t n);
int check_free_space(void);
void fatal_error(void);

#endif
