#include "flightstate.h"
#include "edge_detector.h"
#include "signal_processing.h"
#include "math_ex.h"
#include "signals.h"
#include "events2.h"
#include "logging/log.h"
#include "led.h"

LOG_MODULE_DECLARE(alturia, 3);

DEFINE_EVENT(event_ignition);
DEFINE_EVENT(event_burnout);
DEFINE_EVENT(event_apogee);
DEFINE_EVENT(event_touchdown);
DEFINE_EVENT(event_liftoff);
DEFINE_EVENT(event_pad_idle);
DEFINE_EVENT(event_pad_ready);

DECLARE_SIGNAL(signal_h);
DECLARE_SIGNAL(signal_v_w);
DECLARE_SIGNAL(signal_a_raw);

const float32_t zero = 0.0f;
const float32_t ignition_acceleration_threshold = 2.0f;
const float32_t liftoff_height = 3.0f;

static mission_state_t state;
unsigned int burnout_count, ignition_count;
uint64_t mission_start_time;

struct edge_detector detector_burnout;
struct edge_detector detector_ignition;
struct edge_detector detector_apogee;
struct edge_detector detector_landing;
struct edge_detector detector_height_liftoff;

struct cond_window_data landing_window_data = {
	.target = 0.0f,
	.window_width =5.0f
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

mission_state_t flightstate_get_state()
{
	return state;
}

bool flightstate_inflight()
{
	return (state == STATE_BOOST) ||
		   (state == STATE_COAST) ||
		   (state == STATE_DESCENDING); 
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

void flightstate_init() {
	state = STATE_STARTUP;
	edge_detector_init(&detector_apogee, edge_detector_cond_st, 0);
	edge_detector_init(&detector_burnout, edge_detector_cond_st, 0);
	edge_detector_init(&detector_ignition, edge_detector_cond_gt, 50);
	edge_detector_init(&detector_landing, edge_detector_cond_window, 1000);
	edge_detector_init(&detector_height_liftoff, edge_detector_cond_gt, 50);

	if (signal_a_raw.value.type != type_matrix) {
		LOG_ERR("expected matrix valued type for signal a_r");
	}

	if (signal_v_w.value.type != type_matrix) {
		LOG_ERR("expected matrix type for signal v_w");
	}

	if (signal_h.value.type != type_float32) {
		LOG_ERR("expected float type for signal h");
	}
}

void flightstate_update() {
	int t = k_uptime_get();
	//statemachine tracking flightstate
	if(state == STATE_STARTUP) {
		if (signal_processing_get_state() == SIGNAL_PROCESSING_ACTIVE) {
			state = STATE_PAD_IDLE;
			event2_fire(&event_pad_idle);
			return;
		}
	}
	
	if(state == STATE_PAD_IDLE) {
		event2_fire(&event_pad_ready);
		state = STATE_PAD_READY;
		return;
	}

	/* When rocket is in pad ready state, try to detect liftoff based on pressure height */
	if (state == STATE_PAD_READY) {
		if (edge_detector_update(&detector_height_liftoff, *signal_h.value.value_ptr.type_float32, (void *)&liftoff_height, t)) {
			state = STATE_BOOST;
			event2_fire(&event_ignition);
			event2_fire(&event_liftoff);
			return;
		}
	}

	/* When rocket is in coast phase or in pad ready state, try to detect ignition */
	if (state == STATE_PAD_READY || state == STATE_COAST) {
		if(edge_detector_update(&detector_ignition, mat_get((*(signal_a_raw.value.value_ptr.type_matrix)), 2, 0), (void *)&ignition_acceleration_threshold, t)) {
			edge_detector_reset(&detector_ignition);
			state = STATE_BOOST;
			ignition_count++;
			event2_fire(&event_ignition);
			if (ignition_count == 1) {
				event2_fire(&event_liftoff);
			}
			return;
		}
	}

	/* When rocket is in Boost state, try to detect burnout based on acceleration */
	if (state == STATE_BOOST) {
		if(edge_detector_update(&detector_burnout, mat_get((*(signal_a_raw.value.value_ptr.type_matrix)), 2, 0), (void *)&zero, t)) {
			edge_detector_reset(&detector_burnout);
			state = STATE_COAST;
			burnout_count++;
			event2_fire(&event_burnout);
			return;
		}
	}

	/* If rocket is in coast state and velocity is smaller than 300 m/s, try to detect apogee based on vertical velocity (positive to negative) */
	if (state == STATE_COAST) {
		float32_t v_vertical = mat_get((*(signal_v_w.value.value_ptr.type_matrix)), 2, 0);
		if(v_vertical < 300.f && edge_detector_update(&detector_apogee, v_vertical, (void *)&zero, t)) {
			state = STATE_DESCENDING;
			event2_fire(&event_apogee);
			return;
		}
	}

	/* If rocket is in descending state, try to detect landing based on velocity */
	if (state == STATE_DESCENDING) {
		if (edge_detector_update(&detector_landing, mat_get((*(signal_v_w.value.value_ptr.type_matrix)), 2, 0), &landing_window_data, t)){
			event2_fire(&event_touchdown);
			state = STATE_LANDED;
			return;
		}
	}
}