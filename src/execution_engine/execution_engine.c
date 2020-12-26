#include <zephyr.h>
#include "execution_context.h"
#include "conditions.h"
#include <logging/log.h>
#include "eventtimer.h"
#include "servos.h"
#include "beeper.h"
#include "pyros.h"

LOG_MODULE_REGISTER(execution_engine, CONFIG_LOG_DEFAULT_LEVEL);

K_MSGQ_DEFINE(evt_queue, sizeof(struct system_evt*), CONFIG_EVENT_QUEUE_LENGTH,
	      sizeof(struct event*));

typedef void (action_activate_t) (const struct conf_desc *ctx, const void * data);

static const struct conf_desc *context;

static void action_pyro_set(const struct conf_desc *ctx, const void *data);
static void action_servo_set(const struct conf_desc *ctx, const void *data);
static void action_beep(const struct conf_desc *ctx, const void *data);
static void action_timer_start(const struct conf_desc *ctx, const void *data);
static void action_timer_stop(const struct conf_desc *ctx, const void *data);

static action_activate_t *action_functions[END_ACT] = {
	[PYRO_SET_ACT] = action_pyro_set,
	[SERVO_SET_ACT] = action_servo_set,
	[BEEP_ACT] = action_beep,
	[TIMER_START_ACT] = action_timer_start,
	[TIMER_STOP_ACT] = action_timer_stop,	
};


struct action* get_action(const struct conf_desc *ctx, unsigned int idx) {
	if (idx >= ctx->num_actions) {
		return NULL;
	}

	struct action *act = conf_actions(ctx);
	return act + idx;
}

struct variable* get_var(const struct conf_desc *ctx, int var_idx)
{
	if(var_idx >= ctx->num_vars) {
		return NULL;
	}

	struct variable *vars = conf_vars(ctx);
	vars += var_idx;
	return vars;
}

struct timer_evt* get_timer(const struct conf_desc *ctx, unsigned int timer_number)
{
	struct timer_evt *timer = conf_timer(ctx);
	
	if (ctx->num_timer == 0) {
		return NULL;
	}

	while(true) {
		if (timer->timer_num == timer_number) {
			return timer;
		}

		if (timer == (timer + ctx->num_timer)) {
			return NULL;
		}

		timer += 1;
	}
}

struct burnout_evt* get_burnout(const struct conf_desc *ctx, unsigned int burnout_number)
{
	struct burnout_evt *burnout = conf_burnout(ctx);
	struct burnout_evt *burnout_end = burnout + ctx->num_burnout;

	if (ctx->num_burnout == 0) {
		return NULL;
	}

	while(true) {
		if (burnout->burnout_num == burnout_number) {
			return burnout;
		}

		if (burnout == burnout_end) {
			return NULL;
		}

		burnout += 1;
	}
}

struct ignition_evt* get_ignition_evt(const struct conf_desc *ctx, unsigned int ignition_number)
{
	struct ignition_evt *ignition = conf_ignition(ctx);
	struct ignition_evt *ignition_end = ignition + ctx->num_ignition;

	if (ctx->num_ignition == 0) {
		return NULL;
	}

	while (true) {
		if (ignition->ignition_num == ignition_number) {
			return ignition;
		}

		if (ignition == ignition_end) {
			return NULL;
		}

		ignition += 1;
	}
}

struct timer_start_action *get_timer_start_action(const struct conf_desc *ctx, unsigned int idx)
{
	struct timer_start_action* d = conf_timer_start_action(ctx);
	
	if (idx >= ctx->num_timer_start_actions) {
		return NULL;
	}

	return d + idx;

}

struct timer_stop_action *get_timer_stop_action(const struct conf_desc *ctx, unsigned int idx) {
	struct timer_stop_action *d = conf_timer_stop_action(ctx);

	if (idx >= ctx->num_timer_stop_actions) {
		return NULL;
	}

	return d + idx;
}

struct pyro_set_action *get_pyro_set_action(const struct conf_desc *ctx, unsigned int idx) {
	struct pyro_set_action *d = conf_pyro_set_actions(ctx);

	if (idx >= ctx->num_pyro_set_actions) {
		return NULL;
	}

	return d + idx;
}

struct pyro_read_action *get_pyro_read_action(const struct conf_desc *ctx, unsigned int idx) {
	struct pyro_read_action *d = conf_pyro_read_actions(ctx);

	if (idx >= ctx->num_pyro_read_actions) {
		return NULL;
	}

	return d + idx;
}

struct servo_read_action *get_servo_read_action(const struct conf_desc *ctx, unsigned int idx) {
	struct servo_read_action *d = conf_servo_read_actions(ctx);

	if (idx >= ctx->num_servo_read_actions) {
		return NULL;
	}

	return d + idx;
}

struct servo_set_action *get_servo_set_action(const struct conf_desc *ctx, unsigned int idx) {
	struct servo_set_action *d = conf_servo_set_actions(ctx);

	if (idx >= ctx->num_servo_set_actions) {
		return NULL;
	}

	return d + idx;
}

struct beep_action *get_beep_action(const struct conf_desc *ctx, unsigned int idx) {
	struct beep_action *d = conf_beep_action(ctx);

	if (idx >= ctx->num_beep_actions) {
		return NULL;
	}

	return d + idx;
}

static void action_timer_start(const struct conf_desc *ctx,
							   const void *data)
{
	const struct timer_start_action *timer_data = data;
	struct timer_evt *timer = get_timer(ctx, timer_data->timer_number);
	struct variable *period = get_var(ctx, timer->period);
	struct variable *cyclic = get_var(ctx, timer->cyclic);

	event_timer_start(timer_data->timer_number, period->storage.int_value, cyclic->storage.int_value);
}

static void action_timer_stop(const struct conf_desc *ctx, 
							  const void *data)
{
	const struct timer_stop_action *timer_data = data;
	event_timer_stop(timer_data->timer_number);
}

static void action_beep(const struct conf_desc *ctx, 
					    const void *data)
{
	const struct beep_action *beep_data = data;
	struct variable *duration = get_var(ctx, beep_data->duration);
	struct variable *pitch = get_var(ctx, beep_data->pitch);

	beep(duration->storage.int_value, pitch->storage.int_value);
}

static void action_servo_set(const struct conf_desc *ctx, const void *data)
{
	const struct servo_set_action *servo_data = data;
	struct variable *var = get_var(ctx, servo_data->value);

	if (var == NULL) {
		//error, value not found
	}

	servo_set_us(servo_data->servo_number, var->storage.int_value);
}

static void action_pyro_set(const struct conf_desc *ctx, const void *data)
{
	const struct pyro_set_action *pyro_data = data;
	pyros_fire(pyro_data->pyro_number);
}


void action_call(const struct conf_desc *ctx, const struct action *act)
{
	void *action_data = NULL;

	switch(act->type){
		case TIMER_START_ACT:
			action_data = get_timer_start_action(ctx, act->act_idx);
		break;
		case TIMER_STOP_ACT:
			action_data = get_timer_stop_action(ctx, act->act_idx);
		break;
		case PYRO_SET_ACT:
			action_data = get_pyro_set_action(ctx, act->act_idx);
		break;
		case PYRO_READ_ACT:
			action_data = get_pyro_read_action(ctx, act->act_idx);
		break;
		case SERVO_SET_ACT:
			action_data = get_servo_set_action(ctx, act->act_idx);
		break;
		case SERVO_READ_ACT:
			action_data = get_servo_read_action(ctx, act->act_idx);
		break;
		case BEEP_ACT:
			action_data = get_beep_action(ctx, act->act_idx);
		break;
	}

	if (action_data == NULL) {
		return;
	}

	action_functions[act->type](ctx, action_data);
}
void event_trigger(enum system_evt_type type)
{
	switch (type) {
		case APOGEE_EVT:
		break;
		case BURNOUT_EVT:
		break;
		case TICK_EVT:
		break;
		case TOUCHDOWN_EVT:
		break;
		case TIMER_EVT:
		break;
		case IGNITION_EVT:
		break;
		case BOOT_EVT:
		break;
	}
}

void event_call_actions(const struct system_evt *evt)
{
	for (int i = 0; i <= evt->action_num; i++) {
		struct action *act = get_action(context, i);

		if (act != NULL) {
			action_call(context, act);
		}
	}
}

void event_call_actions_async(struct system_evt *evt)
{
	k_msgq_put(&evt_queue, &evt, K_NO_WAIT);
}

static void async_event_thread()
{
	struct system_evt *evt;
	while (1) {
		int res = k_msgq_get(&evt_queue, &evt, K_FOREVER);
		if (res != 0) {
			LOG_ERR("Error dequeueing event");
			continue;
		}

		event_call_actions(evt);
	}

}

void event_loop(const struct conf_desc *ctx)
{
	context = ctx;
	setup_event_timers(ctx);
	event_trigger(BOOT_EVT);

	while(1) {
		event_trigger(TICK_EVT);
		k_sleep(K_MSEC(100));
	}
}



K_THREAD_DEFINE(event_thread, CONFIG_EVENT_HANDLER_THREAD_STACK_SIZE,
		async_event_thread, NULL, NULL, NULL, 0, 0, 0);
