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

#ifndef __ALTURIA_FLIGHTCTRL_H__
#define __ALTURIA_FLIGHTCTRl_H__

enum flight_state {
        FLS_INIT,
        FLS_PAD,
        FLS_BOOST,
        FLS_ABORT,
        FLS_COAST_SLOW,
        FLS_COAST_FAST,
        FLS_DESCENT,
        FLS_FREE_FALL,
        FLS_LANDED
};


void flightctrl_init();

#endif /* __ALTURIA_FLIGHTCTRL_H__ */