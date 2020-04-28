#include <zephyr.h>
#include "servos.h"
#include "beeper.h"
#include "actions.h"
#include "pyros.h"
#include "eventtimer.h"
#include <logging/log.h>

LOG_MODULE_REGISTER(actions, CONFIG_LOG_DEFAULT_LEVEL);

/* Forward declaration of action functions */
#define ACTION(type, callback) static void callback(void * action_data);
ACTIONS_DESCRIPTION
#undef ACTION

#define ACTION(type, callback) [type] = callback,
static action_activate_t *action_functions[ACTION_END] = {
	ACTIONS_DESCRIPTION
};
#undef ACTION

static void action_timer_start(void *action_data)
{
	struct event_timer_data *data = action_data;
	event_timer_start(data->timer_number, data->count, data->cyclic);
}

static void action_timer_stop(void *action_data)
{
	struct event_timer_data *data = action_data;
	event_timer_stop(data->timer_number);
}

static void action_beep(void *action_data)
{
	struct beep_action_data *data = action_data;

	beep(data->duration, data->pitch);
}

static void action_beeper_set_volume(void *action_data)
{
	struct action_beeper_volume_data *data = action_data;
	beeper_set_volume(data->volume);
}

static void action_servo_set(void *action_data)
{
	struct servo_action_data *data = action_data;
	servo_set_us(data->servo_number, data->position);
}

static void action_pyro_fire(void *action_data)
{
	u8_t *pyro_number = action_data;
	pyros_fire(*pyro_number);
}


void action_call(struct action *act)
{
	action_functions[act->type](act->action_data);
}

void action_init(struct action * action, action_type_t type, void * action_data)
{
	action->type = type;
	action->action_data = action_data;
	action->next = NULL;
}
