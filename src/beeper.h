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

#ifndef ALTURIA__BEEPER__H
#define ALTURIA__BEEPER__H

#include <stdint.h>
#include "alturia.h"

#ifdef ALTURIA_HAS_BEEPER

#define VOLUME 100 /* 0 to 100 */
#define BEEP_FREQUENCY_HZ(n) (1000000000/n)

int beep_off();
int beep_on(int32_t pitch);
int beep(int32_t duration, int32_t pitch);
int beepn(int32_t duration, int32_t count, int32_t pitch);
int beeper_set_volume(uint8_t volume);

#endif
#endif
