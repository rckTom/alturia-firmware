#ifndef ALTURIA_FLIGHT_STATE_H
#define ALTURIA_FLIGHT_STATE_H

#include <zephyr.h>
#include "numeric.h"

typedef enum mission_state {
	STATE_STARTUP,
	STATE_PAD_IDLE,
	STATE_PAD_READY,
	STATE_BOOST,
	STATE_COAST,
	STATE_DESCENDING,
	STATE_LANDED,
	STATE_ERROR,
} mission_state_t;

#define FLIGHT_STATE_VARIABLES \
	VAR(FS_VAR_ALTITUDE, NUMERIC_TYPE_FLOAT) \
	VAR(FS_VAR_SPEED, NUMERIC_TYPE_FLOAT) \
	VAR(FS_VAR_ACC_MAG, NUMERIC_TYPE_FLOAT) \
	VAR(FS_VAR_FLIGHT_STATE, NUMERIC_TYPE_INT) \
	VAR(FS_VAR_BURNOUT_NUM, NUMERIC_TYPE_INT) \
	VAR(FS_VAR_MISSION_TIME, NUMERIC_TYPE_LONG) \
	VAR(FS_VAR_SYSTEM_TIME, NUMERIC_TYPE_LONG) \

#define VAR(name, type) name,
typedef enum flight_state_var {
	FLIGHT_STATE_VARIABLES
} flight_state_var_t;
#undef VAR

numeric_t flightstate_get_var(flight_state_var_t var);
int flightstate_get_burnout_count();
void flightstate_set_burnout_count(int val);

#endif
