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

#ifndef ALTURIA__DAQ__H
#define ALTURIA__DAQ__H

#include "drivers/sensor.h"

extern struct sensor_value acc_sample[];
extern struct sensor_value gyr_sample[];
extern struct sensor_value press_sample;

int daq_sync();
int daq_start();

#endif /* ALTURIA__DAQ__H */
