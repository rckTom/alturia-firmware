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

#ifndef ALTURIA__ATMOSPHERE__H
#define ALTURIA__ATMOSPHERE__H

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

#endif /* ALTURIA__ATMOSPHERE__H */
