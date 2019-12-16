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

#ifndef __BEEPER_H__
#define __BEEPER_H__

#include <zephyr.h>

#define VOLUME 5 /* 0 to 100 */

int beep_off();
int beep_on(int32_t pitch);
int beep(s32_t duration, s32_t pitch);
int beepn(s32_t duration, s32_t count, s32_t pitch);
int set_volume(uint8_t volume);

#endif
