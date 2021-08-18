#ifndef ALTURIA__GENERIC__H
#define ALTURIA__GENERIC__H

#include "stdint.h"
#include "math_ex.h"

#define GENERICS \
    GENERIC(uint8, uint8_t) \
    GENERIC(uint16, uint16_t) \
    GENERIC(uint32, uint32_t) \
    GENERIC(uint64, uint64_t) \
    GENERIC(int8, int8_t) \
    GENERIC(int16, int16_t) \
    GENERIC(int32, int32_t) \
    GENERIC(int64, int64_t) \
    GENERIC(float32, float32_t) \
    GENERIC(string, char) \
    GENERIC(matrix, mat) \

struct generic_ptr {
    #define GENERIC(name, ctype) type_##name,
    enum {
        GENERICS
    } type;
    #undef GENERIC

    #define GENERIC(name, ctype) ctype *name;
    union {
        GENERICS
    } value_ptr;
    #undef GENERIC
};

#define GENERIC(name, ctype) struct generic_ptr generic_ptr_from_##name(ctype *name);
GENERICS
#undef GENERIC

#endif