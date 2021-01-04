#ifndef ALTURIA_EVENTTIMER_H
#define ALTURIA_EVENTTIMER_H

#include <zephyr.h>
#include <stdint.h>


int event_timer_start(uint8_t timer_number, uint32_t count, bool cyclic);
int event_timer_stop(uint8_t timer_number);
int event_timer_set_callback(void (*clbk)(int));
void setup_event_timers();

#endif
