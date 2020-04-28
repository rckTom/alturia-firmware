#include "numeric.h"
#include <string.h>

static bool type_equal(const numeric_t *a, const numeric_t *b)
{
	return (a->type == b->type);
}

int numeric_goe(const numeric_t *a, const numeric_t *b, bool *res)
{
	if (!type_equal(a,b)) {
		return -1;
	}

	switch(a->type) {
		case NUMERIC_TYPE_FLOAT:
			*res = a->asFloat >= b->asFloat;
			break;
		case NUMERIC_TYPE_INT:
			*res = a->asInt >= b->asInt;
			break;
		case NUMERIC_TYPE_LONG:
			*res = a->asLong >= b->asLong;
			break;
		case NUMERIC_TYPE_UINT:
			*res = a->asUInt >= b->asUInt;
			break;
		case NUMERIC_TYPE_ULONG:
			*res = a->asULong >= b->asULong;
			break;
		case NUMERIC_TYPE_INVALID:
			return -1;
	}

	return 0;
}

int numeric_gt(const numeric_t *a, const numeric_t *b, bool *res)
{
	if (!type_equal(a,b)) {
		return -1;
	}

	switch(a->type) {
		case NUMERIC_TYPE_FLOAT:
			*res = a->asFloat > b->asFloat;
			break;
		case NUMERIC_TYPE_INT:
			*res = a->asInt > b->asInt;
			break;
		case NUMERIC_TYPE_LONG:
			*res = a->asLong > b->asLong;
			break;
		case NUMERIC_TYPE_UINT:
			*res = a->asUInt > b->asUInt;
			break;
		case NUMERIC_TYPE_ULONG:
			*res = a->asULong > b->asULong;
		case NUMERIC_TYPE_INVALID:
			return -1;
	}

	return 0;
}

int numeric_loe(const numeric_t *a, const numeric_t *b, bool *res)
{
	if (!type_equal(a,b)) {
		return -1;
	}

	switch(a->type) {
		case NUMERIC_TYPE_FLOAT:
			*res = a->asFloat <= b->asFloat;
			break;
		case NUMERIC_TYPE_INT:
			*res = a->asInt <= b->asInt;
			break;
		case NUMERIC_TYPE_LONG:
			*res = a->asLong <= b->asLong;
			break;
		case NUMERIC_TYPE_UINT:
			*res = a->asUInt <= b->asUInt;
			break;
		case NUMERIC_TYPE_ULONG:
			*res = a->asULong <= b->asULong;
		case NUMERIC_TYPE_INVALID:
			return -1;
	}

	return 0;
}

int numeric_lt(const numeric_t *a, const numeric_t *b, bool *res)
{
	if (!type_equal(a,b)) {
		return -1;
	}

	switch(a->type) {
		case NUMERIC_TYPE_FLOAT:
			*res = a->asFloat < b->asFloat;
			break;
		case NUMERIC_TYPE_INT:
			*res = a->asInt < b->asInt;
			break;
		case NUMERIC_TYPE_LONG:
			*res = a->asLong < b->asLong;
			break;
		case NUMERIC_TYPE_UINT:
			*res = a->asUInt < b->asUInt;
			break;
		case NUMERIC_TYPE_ULONG:
			*res = a->asULong < b->asULong;
		case NUMERIC_TYPE_INVALID:
			return -1;
	}

	return 0;
}

int numeric_eq(const numeric_t *a, const numeric_t *b, bool *res)
{
	if (!type_equal(a,b)) {
		return -1;
	}

	switch(a->type) {
		case NUMERIC_TYPE_FLOAT:
			*res = a->asFloat == b->asFloat;
			break;
		case NUMERIC_TYPE_INT:
			*res = a->asInt == b->asInt;
			break;
		case NUMERIC_TYPE_LONG:
			*res = a->asLong == b->asLong;
			break;
		case NUMERIC_TYPE_UINT:
			*res = a->asUInt == b->asUInt;
			break;
		case NUMERIC_TYPE_ULONG:
			*res = a->asULong == b->asULong;
		case NUMERIC_TYPE_INVALID:
			return -1;
	}

	return 0;
}
