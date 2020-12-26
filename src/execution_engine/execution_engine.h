#ifndef ALTURIA_EXECUTION_ENGINE_H
#define ALTURIA_EXECUTION_ENGINE_H

#include "execution_context.h"

void action_call(const struct conf_desc *ctx, const struct action *act);
void event_call_actions(const struct system_evt *evt);
void event_call_actions_async(const struct system_evt *evt);
struct event *event_get(enum system_evt_type evt_type);
void event_loop(const struct conf_desc *ctx);

#endif