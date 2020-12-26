#ifndef ALTURIA_STRUCTCONF_EVT_H
#define ALTURIA_STRUCTCONF_EVT_H

#include <stdint.h>
#include "generic.h"
#include "execution_context.h"

enum system_evt_type {
	TIMER_EVT = 0,
	BOOT_EVT = 1,
	TICK_EVT = 2,
	APOGEE_EVT = 3,
	IGNITION_EVT = 4,
	TOUCHDOWN_EVT = 5,
	BURNOUT_EVT = 6,
};

struct system_evt {
	uint8_t type;
	uint32_t action_start;
	uint32_t action_num;
} __attribute__((packed));

struct timer_evt {
	struct system_evt evt;
	uint8_t timer_num;
	uint8_t cyclic;
	value_ref_t period;
} __attribute__((packed));

struct burnout_evt {
	struct system_evt evt;
	uint8_t burnout_num;
} __attribute__((packed));

struct ignition_evt {
	struct system_evt evt;
	uint8_t ignition_num;
} __attribute__((packed));

struct touchdown_evt {
	struct system_evt evt;
} __attribute__((packed));

struct boot_evt {
	struct system_evt evt;
} __attribute__((packed));

struct tick_evt {
	struct system_evt evt;
} __attribute__((packed));

struct apogee_evt {
	struct system_evt evt;
} __attribute__((packed));


#endif
