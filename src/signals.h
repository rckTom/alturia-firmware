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

#define DEFINE_SIGNAL(signal_name, ptr) \
         STRUCT_SECTION_ITERABLE(signal, signal_name) = {\
                 .name = #signal_name,\
                 .value = ptr, \
         } 

#define DECLARE_SIGNAL(name) \
    extern struct signal name;

struct signal{
    const char* name;
    struct generic_ptr value;
    size_t len;
};

#endif