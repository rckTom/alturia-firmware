#ifndef ALTURIA_FLIGHT_STATE_H
#define ALTURIA_FLIGHT_STATE_H

#include <zephyr/kernel.h>
#include "numeric.h"

typedef enum mission_state {
	STATE_STARTUP,
	STATE_SAFE,
	STATE_PAD_IDLE,
	STATE_PAD_READY,
	STATE_BOOST,
	STATE_COAST,
	STATE_DESCENDING,
	STATE_LANDED,
	STATE_ERROR,
} mission_state_t;

mission_state_t flightstate_get_state();
bool flightstate_inflight();
int flightstate_get_ignition_count();
int flightstate_get_burnout_count();
void flightstate_set_burnout_count(int val);
void flightstate_update();
void flightstate_init();

#endif
