/*
 * Alturia Firmware - The firmware for the alturia flight computer
 *
 * Copyright (c) Thomas Schmid, 2021
 *
 * Authors:
 *  Thomas Schmid <tom@lfence.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 3.  See
 * the COPYING file in the top-level directory.
 */

#ifndef ALTURIA__SIGNALS__H
#define ALTURIA__SIGNALS__H

#include "stdlib.h"
#include "generic.h"

struct signal{
    const char* name;
    struct generic_ptr value;
    size_t len;
};

int signal_insert(const char *name, struct generic_ptr data);
int signal_update(const char *name, struct generic_ptr data);
struct generic_ptr signal_get(const char *name);

#endif