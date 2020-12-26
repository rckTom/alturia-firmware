#ifndef ALTURIA_EXECUTION_ENGINE_ACTIONS_H
#define ALTURIA_EXECUTION_ENGINE_ACTIONS_H

#include <stdint.h>
#include "generic.h"

struct conf_desc;

enum action_type {
	PYRO_SET_ACT = 0,
	PYRO_READ_ACT = 1,
	SERVO_SET_ACT = 2,
	SERVO_READ_ACT = 3,
	BEEP_ACT = 4,
	TIMER_START_ACT = 5,
	TIMER_STOP_ACT = 6,
	END_ACT
};

struct action {
	uint32_t type;
	uint32_t act_idx;
} __attribute__((packed));

struct pyro_set_action {
	uint8_t pyro_number;
} __attribute__((packed));

struct pyro_read_action {
	uint8_t pyro_number;
	value_ref_t value;
} __attribute__((packed));

struct servo_set_action {
	uint8_t servo_number;
	value_ref_t value;
} __attribute__((packed));

struct servo_read_action {
	uint8_t servo_number;
	value_ref_t value;
} __attribute__((packed));

struct beep_action {
	value_ref_t pitch;
	value_ref_t duration;
} __attribute__((packed));

struct timer_start_action {
	uint8_t timer_number;
} __attribute__((packed));

struct timer_stop_action {
	uint8_t timer_number;
} __attribute__((packed));


#endif
