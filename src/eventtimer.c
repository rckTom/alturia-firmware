#include "util.h"
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

LOG_MODULE_REGISTER(eventtimer, CONFIG_LOG_DEFAULT_LEVEL);

struct event_timer {
	struct k_timer timer;
};

void (*clbk)(int) = NULL;
struct event_timer event_timers[16];

static void event_timer_expr(struct k_timer *timer)
{
	struct event_timer *evt_timer =
	    CONTAINER_OF(timer, struct event_timer, timer);

	if (clbk) {
		int idx = evt_timer - event_timers;
		clbk(idx);
	}
}

int event_timer_start(uint8_t timer_number, uint32_t count, bool cyclic)
{
	if (timer_number >= ARRAY_SIZE(event_timers)) {
		LOG_ERR("Timer number to large");
		return -EINVAL;
	}

	struct event_timer *evt_timer = event_timers + timer_number;

	if (k_timer_remaining_get(&evt_timer->timer) != 0) {
		LOG_WRN("Timer %d already running. Remaining %d", timer_number, k_timer_remaining_get(&evt_timer->timer));
		return -EINVAL;
	}

	k_timer_start(&evt_timer->timer, K_MSEC(count),
		      cyclic ? K_MSEC(count) : K_MSEC(0));
	return 0;
}

int event_timer_stop(uint8_t timer_number)
{
	if (timer_number >= ARRAY_SIZE(event_timers)) {
		LOG_ERR("Timer number to large");
		return -EINVAL;
	}

	k_timer_stop(&(event_timers + timer_number)->timer);

	return 0;
}

void event_timer_set_callback(void (*callback) (int))
{
	clbk = callback;
}

static int event_timer_init()
{
	for (int i = 0; i < ARRAY_SIZE(event_timers); i++) {
		k_timer_init(&(event_timers + i)->timer, event_timer_expr,
			     NULL);
	}
	return 0;
}

SYS_INIT(event_timer_init, APPLICATION, 0);
