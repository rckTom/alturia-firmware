#ifndef ALTURIA_NUMERIC_H
#define ALTURIA_NUMERIC_H

#include <stdbool.h>

#define DESCRIPTION \
	TYPE(NUMERIC_TYPE_FLOAT, float, asFloat) \
	TYPE(NUMERIC_TYPE_INT, int, asInt) \
	TYPE(NUMERIC_TYPE_LONG, long, asLong) \
	TYPE(NUMERIC_TYPE_UINT, unsigned int, asUInt) \
	TYPE(NUMERIC_TYPE_ULONG, unsigned long, asULong)

#define TYPE(type, ctype, member) type,
typedef enum numeric_type {
	DESCRIPTION
	NUMERIC_TYPE_INVALID
} numeric_type_t;
#undef TYPE

#define TYPE(type, ctype, member) ctype member;
typedef struct numeric_val {
	numeric_type_t type;
	union {
		DESCRIPTION
	};
#undef TYPE

} numeric_t;

int numeric_goe(const numeric_t *a, const numeric_t *b, bool *res);
int numeric_gt(const numeric_t *a, const numeric_t *b, bool *res);
int numeric_loe(const numeric_t *a, const numeric_t *b, bool *res);
int numeric_lt(const numeric_t *a, const numeric_t *b, bool *res);
int numeric_eq(const numeric_t *a, const numeric_t *b, bool *res);

#endif
