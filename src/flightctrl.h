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

#ifndef ALTURIA__FLIGHTCTRL__H
#define ALTURIA__FLIGHTCTRl__H

enum flight_state {
        FLS_INIT,
        FLS_PAD,
        FLS_BOOST,
        FLS_ABORT,
        FLS_COAST,
        FLS_DESCENT,
        FLS_FREE_FALL,
        FLS_LANDED
};


void flightctrl_init();

#endif /* ALTURIA__FLIGHTCTRL__H */
