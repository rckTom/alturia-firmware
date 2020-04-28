#ifndef ALTURIA_EVENTTIMER_H
#define ALTURIA_EVENTTIMER_H

#include <zephyr.h>

int event_timer_start(u8_t timer_number, u32_t count, bool cyclic);
int event_timer_stop(u8_t timer_number);
void setup_event_timers();

#endif
