#include "flightstate.h"

static int burnout_count;
static long mission_start_time;
static mission_state_t mission_state;


#define VAR(name, type) [name] = type,
static const numeric_type_t flight_state_var_num_type[] = {
	FLIGHT_STATE_VARIABLES
};
#undef VAR

numeric_t flightstate_get_var(flight_state_var_t var)
{
	numeric_t res;
	res.type = flight_state_var_num_type[var];

	switch (var) {
		case FS_VAR_SYSTEM_TIME:
			res.asLong = k_uptime_get();
			break;
		case FS_VAR_BURNOUT_NUM:
			res.asInt = burnout_count;
			break;
		case FS_VAR_MISSION_TIME:
			res.asLong = k_uptime_get() - mission_start_time;
			break;
		case FS_VAR_FLIGHT_STATE:
			res.asInt = mission_state;
			break;
		/* ToDo
		 * Implementation for remaining flight state variables
		 */
	}

	return res;
}

void flightstate_set_burnout_count(int val)
{
	burnout_count = val;
}

int flightstate_get_burnout_count()
{
	return burnout_count;
}

void flightstate_set_mission() {
	mission_start_time = k_uptime_get();
}
