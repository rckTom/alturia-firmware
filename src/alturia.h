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

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include "rocket/flightstate.h"

#if DT_NODE_EXISTS(DT_ALIAS(pressure_sensor))
    #define ALTURIA_HAS_PRESSURE_SENSOR
#endif
#if DT_NODE_EXISTS(DT_ALIAS(acc_sensor))
    #define ALTURIA_HAS_ACC_SENSOR
#endif
#if DT_NODE_EXISTS(DT_ALIAS(gyro_sensor))
    #define ALTURIA_HAS_GYRO_SENSOR
#endif

#if DT_NODE_EXISTS(DT_NODELABEL(beeper))
    #define ALTURIA_HAS_BEEPER
#endif

#define ALTURIA_FLASH_MP                    "/lfs"
#define ALTURIA_MINIMUM_FREE_SPACE	    1024*3

void __attribute__((noreturn)) panic(void);
void start_delay(int delay);
int next_datalog_path(char *path, size_t n);
int check_free_space(void);
void fatal_error(void);



#endif
