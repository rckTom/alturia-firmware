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

#ifndef __ALTURIA_ATMOSPHERE_H__
#define __ALTURIA_ATMOSPHERE_H__

void init_atmosphere();
float calc_height(float pressure);
float calc_pressure(float height);
float calc_temperature(float height);

#endif /* __ALTURIA_ATMOSPHERE_H__ */
