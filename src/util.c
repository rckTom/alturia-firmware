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

#include "util.h"

char* z_fgets(char* s, int n, struct fs_file_t *file)
{
    int c;
    int res;
    char* cs;
    cs = s;

    while (--n > 0 && (res = fs_read(file, &c, 1) == 1))
    {
    // put the input char into the current pointer position, then increment it
    // if a newline entered, break
    if ((*cs++ = c) == '\n')
        break;
    }

    *cs = '\0';
    return (res <= 0 && cs == s) ? NULL : s;
}

float sensor_value_to_float(struct sensor_value *sval)
{
	return ((float) sval->val1 +
	       ((float) sval->val2) / 1000000.0f);
}
