#ifndef ALTURIA_ACTIONS_H
#define ALTURIA_ACTIONS_H

#include <zephyr.h>

typedef void(action_activate_t)(void *data);

#define ACTIONS_DESCRIPTION                                                    \
	ACTION(ACTION_TIMER_START, action_timer_start)                         \
	ACTION(ACTION_TIMER_STOP, action_timer_stop)                           \
	ACTION(ACTION_BEEP, action_beep)                                       \
	ACTION(ACTION_BEEPER_SET_VOLUME, action_beeper_set_volume)             \
	ACTION(ACTION_SERVO_SET, action_servo_set)                             \
	ACTION(ACTION_PYRO_FIRE, action_pyro_fire)

#define ACTION(type, callback) type,
typedef enum {
	ACTIONS_DESCRIPTION ACTION_END,
} action_type_t;
#undef ACTION

struct action {
	action_type_t type;
	void *action_data;
	struct action *next;
};

struct event_timer_data {
	int32_t count;
	uint8_t timer_number;
	bool cyclic;
};

struct beep_action_data {
	int32_t duration;
	int32_t pitch;
};

struct servo_action_data {
	uint8_t servo_number;
	uint32_t position;
};

struct action_beeper_volume_data {
	uint8_t volume;
};

void action_call(struct action *act);
void action_init(struct action *action, action_type_t type, void *action_data);
#endif
