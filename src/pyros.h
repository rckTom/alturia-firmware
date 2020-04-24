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

#ifndef ALTURIA__PYROS__H
#define ALTURIA__PYROS__H

int pyros_fire(unsigned int pyro);
void get_pyro_status(unsigned int pyro);

#endif /* ALTURIA__PYROS__H */
