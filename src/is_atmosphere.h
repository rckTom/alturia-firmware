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

void isa_init();
float isa_calc_height(float pressure);
float isa_calc_pressure(float height);
float isa_calc_temperature(float height);

#endif /* __ALTURIA_ATMOSPHERE_H__ */
