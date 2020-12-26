#ifndef ALTURIA_STRUCTCONF_VARIABLE_H
#define ALTURIA_STRUCTCONF_VARIABLE_H

#include "generic.h"
#include "stdint.h"

enum variable_type {
	INT = 0,
	FLOAT = 1,
	VARIABLE_TYPE_MAKE_8BIT = 0xFF
};

struct variable {
	uint8_t constant;
	uint8_t type;
	union {
		uint32_t int_value;
		float float_value;
	} storage;
} __attribute__((packed));



#endif

