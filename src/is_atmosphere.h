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

/**
 * isa_init() - Initialize the layer table
 *
 * Initilizes the layer table layers based on the first entry of the table
 */
void isa_init();

/**
 * isa_calc_height() - Calculate altitude based on pressure
 * @p: pressure in Pascal
 *
 * Return: altitude above sea level in meters
 */
float isa_calc_height(float pressure);

/**
 * isa_calc_pressure() - Calculate pressure at given altitude
 * @h: altitude above sea level in meters
 *
 * Return: pressure in Pascal
 */
float isa_calc_pressure(float height);

/**
 * isa_calc_temperature() - Calculate temperature at given altitude
 * @h: altitude above sea level in meters
 *
 * Return: temperature in kelvin
 */
float isa_calc_temperature(float height);

#endif /* __ALTURIA_ATMOSPHERE_H__ */
