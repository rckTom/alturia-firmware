#include "flightstate.h"
#include "edge_detector.h"
#include "signal_processing.h"
#include "math_ex.h"
#include "signals.h"
#include "events2.h"

DEFINE_EVENT(event_ignition);
DEFINE_EVENT(event_burnout);
DEFINE_EVENT(event_apogee);
DEFINE_EVENT(event_touchdown);

const float32_t zero = 0.0f;
const float32_t ignition_acceleration_threshold = 2.0f;

static mission_state_t state;
unsigned int burnout_count, ignition_count;
uint64_t mission_start_time;

struct edge_detector detector_burnout;
struct edge_detector detector_ignition;
struct edge_detector detector_apogee;
struct edge_detector detector_landing;

struct cond_window_data landing_window_data = {
	.target = 0.0f,
	.window_width = 1.0f
};

static struct generic_ptr altitude, vertical_velocity, acceleration;

/* Ignition:
 * Acceleration larger than 2g for more than 100 ms
 */

/* Apogee:
 * Speed goes negative during coast and when speed is smaller < 300 m/s
 */

/* Burnout:
 * Acceleration goes negative during boost
 */

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

void flightstate_init() {
	edge_detector_init(&detector_apogee, edge_detector_cond_st, 0);
	edge_detector_init(&detector_burnout, edge_detector_cond_st, 0);
	edge_detector_init(&detector_ignition, edge_detector_cond_gt, 0);
	edge_detector_init(&detector_landing, edge_detector_cond_window, 1000);

	altitude = signal_get("h");
	vertical_velocity = signal_get("v_w");
	acceleration = signal_get("a_r");

	if (acceleration.type != type_matrix) {
		LOG_ERR("expected matrix valued type for signal a_r");
	}

	if (vertical_velocity.type != type_matrix) {
		LOG_ERR("expected matrix type for signal v_w");
	}

	if (acceleration.type != type_matrix) {
		LOG_ERR("expected matrix type for signal a_m");
	}
}

void flightstate_update() {
	int t = k_uptime_get();
	//statemachine tracking flightstate
	if(state == STATE_STARTUP) {
		if (
			signal_processing_get_state() == SIGNAL_PROCESSING_ACTIVE
		) {
			state == STATE_PAD_IDLE;
		}
	}

	if(state == STATE_PAD_IDLE) {
		
	}


	if (state == STATE_PAD_READY || state == STATE_COAST) {
		if(edge_detector_update(&detector_ignition, mat_get((*(acceleration.value_ptr.matrix)), 2, 0), &ignition_acceleration_threshold, t)) {
			edge_detector_reset(&detector_ignition);
			state = STATE_BOOST;
			ignition_count++;
			event2_fire(&event_ignition);
		}
	}

	if (state == STATE_BOOST) {
		if(edge_detector_update(&detector_burnout, mat_get((*(acceleration.value_ptr.matrix)), 2, 0), &zero, t)) {
			edge_detector_reset(&detector_burnout);
			state = STATE_COAST;
			burnout_count++;
			event2_fire(&event_burnout);
		}
	}

	if (state == STATE_COAST) {
		if(edge_detector_update(&detector_apogee, mat_get((*(vertical_velocity.value_ptr.matrix)), 2, 0), &zero, t)) {
			state = STATE_DESCENDING;
			event2_fire(&event_apogee);
		}
	}

	if (state == STATE_DESCENDING) {
		if (edge_detector_update(&detector_landing, mat_get((*(vertical_velocity.value_ptr.matrix)), 2, 0), &landing_window_data, t)){
			event2_fire(&event_touchdown);
		}
	}
}