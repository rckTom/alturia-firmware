#include <zephyr.h>
#include "events.h"
#include "actions.h"
#include "conditions.h"
#include <logging/log.h>
#include "eventtimer.h"

LOG_MODULE_REGISTER(event_system, CONFIG_LOG_DEFAULT_LEVEL);

K_MSGQ_DEFINE(evt_queue, sizeof(struct event*), CONFIG_EVENT_QUEUE_LENGTH,
	      sizeof(struct event*));

static struct event *events[EVT_ENUM_END];

void event_trigger(event_type type)
{
	struct event *evt = events[type];

	if (evt == NULL) {
		return;
	}

	while(1) {
		event_call_actions_async(evt);

		if (evt->next == NULL) {
			break;
		}

		evt = evt->next;
	}
}

void event_call_actions(const struct event *evt)
{
	if (evt == NULL) {
		return;
	}

	struct action *act = evt->action_list;

	if (act == NULL) {
		return;
	}

	if(!conditions_evaluate(evt->evt_conditions)) {
		return;
	}

	while(1) {
		action_call(act);
		if(act->next == NULL) {
			break;
		}

		act = act -> next;
	}
}

void event_call_actions_async(struct event *evt)
{
	k_msgq_put(&evt_queue, &evt, K_NO_WAIT);
}

void event_action_append(struct event *evt, struct action *act) {
	if (evt->action_list == NULL) {
		evt->action_list = act;
		return;
	}

	struct action *head = evt->action_list;

	while(head->next != NULL) {
		head = head->next;
	}

	head->next = act;
	head->next->next = NULL;
}

void event_append(struct event *evt)
{
	if (events[evt->evt_type] == NULL) {
		events[evt->evt_type] = evt;
		return;
	}

	struct event *head = events[evt->evt_type];

	while(head->next != NULL) {
		head = head->next;
	}

	head->next = evt;
	head->next->next = NULL;
}

struct event* event_get(event_type evt_type)
{
	return events[evt_type];
}

static void async_event_thread()
{
	struct event *evt;
	while (1) {
		int res = k_msgq_get(&evt_queue, &evt, K_FOREVER);
		if (res != 0) {
			LOG_ERR("Error dequeueing event");
			continue;
		}

		event_call_actions(evt);
	}

}

void event_initialize(struct event *evt, event_type type, void * param, condition_t *cond)
{
	evt->evt_conditions = cond;
	evt->param = param;
	evt->evt_type = type;
	evt->next = NULL;
	evt->action_list = NULL;

}

void event_loop()
{
	setup_event_timers();
	event_trigger(EVT_BOOT);

	while(1) {
		event_trigger(EVT_TICK);
		k_sleep(K_MSEC(100));
	}
}

K_THREAD_DEFINE(event_thread, CONFIG_EVENT_HANDLER_THREAD_STACK_SIZE,
		async_event_thread, NULL, NULL, NULL, 0, 0, 0);
