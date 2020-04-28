#include "events.h"
#include "util.h"
#include <zephyr.h>
#include <init.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(eventtimer, CONFIG_LOG_DEFAULT_LEVEL);

struct event_timer {
	struct event *evt;
	struct k_timer timer;
};

struct event_timer event_timers[16];

static void event_timer_expr(struct k_timer *timer)
{
	struct event_timer *evt_timer = CONTAINER_OF(timer, struct event_timer,
						     timer);

	if (evt_timer->evt == NULL) {
		LOG_ERR("Event timer expired but no reference to event found");
		return;
	}

	event_call_actions_async(evt_timer->evt);
}


int event_timer_start(u8_t timer_number, u32_t count, bool cyclic)
{
	if (timer_number >= ARRAY_SIZE(event_timers)) {
		LOG_ERR("Timer number to large");
		return -EINVAL;
	}

	struct event_timer *evt_timer = event_timers + timer_number;

	if (k_timer_remaining_get(&evt_timer->timer) != 0) {
		LOG_WRN("Timer already running");
		return -EINVAL;
	}

	k_timer_start(&evt_timer->timer, K_MSEC(count),
		      cyclic ? K_MSEC(count) : K_MSEC(0));
	return 0;
}

int event_timer_stop(u8_t timer_number)
{
	if (timer_number >= ARRAY_SIZE(event_timers)) {
		LOG_ERR("Timer number to large");
		return -EINVAL;
	}

	k_timer_stop(&(event_timers + timer_number)->timer);

	return 0;
}

int setup_event_timers() {
	struct event* evt = event_get(EVT_TIMER_EXPR);

	if (evt == NULL) {
		return -EINVAL;
	}

	while (1) {
		timer_event_data_t *data = evt->param;

		if(data->timer_number >= ARRAY_SIZE(event_timers)) {
			LOG_ERR("timer number to large");
			return -EINVAL;
		}

		event_timers[data->timer_number].evt = evt;

		if (evt->next == NULL) {
			break;
		}

		evt = evt->next;
	}

	return 0;
}

static int event_timer_init() {
	for (int i = 0; i < ARRAY_SIZE(event_timers); i++) {
		k_timer_init(&(event_timers + i)->timer, event_timer_expr,
			     NULL);
	}
	return 0;
}

SYS_INIT(event_timer_init, APPLICATION, 0);
