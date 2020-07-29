#ifndef ALTURIA_EVENTS_H
#define ALTURIA_EVENTS_H

#include "conditions.h"
#include <zephyr.h>

typedef enum events {
	EVT_BOOT,
	EVT_LIFTOFF,
	EVT_ASCENT_HEIGHT,
	EVT_ASCENT_RATE,
	EVT_APOGEE,
	EVT_DESCENT_HEIGHT,
	EVT_DESCENT_RATE,
	EVT_LANDING,
	EVT_TIMER_EXPR,
	EVT_TICK,
	EVT_ENUM_END,
} event_type;

typedef struct timer_event_data {
	uint8_t timer_number;
} timer_event_data_t;

typedef struct ascent_height_event_data {
	float ascent_height;
} ascent_height_event_data_t;

typedef struct ascent_rate_event_data {
	float ascent_rate;
} ascent_rate_event_data_t;

typedef struct descent_height_event_data {
	float descent_height;
} descent_height_event_data_t;

typedef struct descent_rate_event_data {
	float descent_rate;
} desecent_rate_event_data_t;

struct event {
	event_type evt_type;
	void *param;
	condition_t *evt_conditions;
	struct action *action_list;
	struct event *next;
};

void event_call_actions(const struct event *evt);
void event_call_actions_async(struct event *evt);
int event_check_conditions(const struct event *evt);
void event_action_append(struct event *evt, struct action *act);
void event_append(struct event *evt);
struct event *event_get(event_type evt_type);
void event_initialize(struct event *evt, event_type type, void *param,
		      condition_t *cond);
void event_loop();

#endif
