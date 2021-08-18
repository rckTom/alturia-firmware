#include "generic.h"

/* Build functions */
#define GENERIC(name, ctype)\
struct generic_ptr generic_ptr_from_##name(ctype *name) { \
    struct generic_ptr ptr = { \
        .type = type_##name, \
        .value_ptr = { \
            .name = name \
        } \
    }; \
    return ptr;\
}
GENERICS
#undef GENERIC