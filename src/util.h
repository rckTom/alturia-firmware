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

#ifndef __ALTURIA_UTIL_H__
#define __ALTURIA_UTIL_H__

#include <fs/fs.h>

char* z_fgets(char* s, int n, struct fs_file_t *file);

#endif
