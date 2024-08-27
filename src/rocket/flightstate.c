#include "flightstate.h"
#include "edge_detector.h"
#include "signal_processing.h"
#include "math_ex.h"
#include "signals.h"
#include "events2.h"
#include <zephyr/logging/log.h>
#include <zephyr/smf.h>
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
DECLARE_SIGNAL(signal_a_w);

struct state_machine_ctx {
	struct smf_ctx ctx;
	int64_t t;
} smf_context;

const float32_t zero = 0.0f;
const float32_t ignition_acceleration_threshold = 20.0f;
const float32_t liftoff_height = 5.0f;

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

/* Forward declaration of state table */
static const struct smf_state states[];

static bool acc_ignition_detector(int64_t t) {
	if(edge_detector_update(&detector_ignition, mat_get((*(signal_a_w.value.value_ptr.type_matrix)), 2, 0), (void *)&ignition_acceleration_threshold, t)) {
		edge_detector_reset(&detector_ignition);
		return true;
	}
	return false;
}
/*
 * Startup State
 */
static void startup_run(void *ctx)
{
	if (signal_processing_get_state() == SIGNAL_PROCESSING_ON_GROUND) {
		smf_set_state(SMF_CTX(ctx), &states[STATE_PAD_IDLE]);
	}
}

/*
 * State Pad Idle
 */
static void pad_idle_run(void *ctx)
{
	smf_set_state(SMF_CTX(ctx), &states[STATE_PAD_READY]);
}

static void pad_idle_entry(void *ctx){
	event2_fire(&event_pad_idle);
}
static void pad_idle_exit(void *ctx) {}

/*
 * State Pad Ready
 */
static void pad_ready_run(void *ctx) {
	struct state_machine_ctx *udata = (struct state_machine_ctx *)ctx;

	/* Try to detect liftoff based on pressure height or acceleration*/
	if (edge_detector_update(&detector_height_liftoff, *signal_h.value.value_ptr.type_float32, (void *)&liftoff_height, udata->t) ||
	    acc_ignition_detector(udata->t)) {
		smf_set_state(SMF_CTX(ctx), &states[STATE_BOOST]);
	}
}
static void pad_ready_entry(void *ctx) {
	event2_fire(&event_pad_ready);
}

/*
 * State Boost
 */
static void boost_run(void *ctx) {
	struct state_machine_ctx *udata = (struct state_machine_ctx *)ctx;

	if(edge_detector_update(&detector_burnout, mat_get((*(signal_a_w.value.value_ptr.type_matrix)), 2, 0), (void *)&zero, udata->t)) {
		edge_detector_reset(&detector_burnout);
		smf_set_state(SMF_CTX(ctx), &states[STATE_COAST]);
	}
}
static void boost_entry(void *ctx) {
	ignition_count++;
	event2_fire(&event_ignition);
	if (ignition_count == 1) {
		event2_fire(&event_liftoff);
	}
}
static void boost_exit(void *ctx) {
	burnout_count++;
	event2_fire(&event_burnout);
}

/*
 * State Coast
 */
static void coast_run(void *ctx){
	struct state_machine_ctx *udata = (struct state_machine_ctx *)ctx;

	/* Try to detect apogee */
	float32_t v_vertical = mat_get((*(signal_v_w.value.value_ptr.type_matrix)), 2, 0);
	if(v_vertical < 300.f && edge_detector_update(&detector_apogee, v_vertical, (void *)&zero, udata->t)) {
		event2_fire(&event_apogee);
		smf_set_state(SMF_CTX(ctx), &states[STATE_DESCENDING]);
		return;
	}

	/* Try to detect another boost */
	if (acc_ignition_detector(udata->t)) {
		smf_set_state(SMF_CTX(ctx), &states[STATE_BOOST]);
	}

}

/*
 * State Descending
 */
static void descending_run(void *ctx) {
	struct state_machine_ctx *udata = (struct state_machine_ctx *)ctx;
	if (edge_detector_update(&detector_landing, mat_get((*(signal_v_w.value.value_ptr.type_matrix)), 2, 0), &landing_window_data, udata->t)){
		event2_fire(&event_touchdown);
		smf_set_state(ctx, &states[STATE_LANDED]);
	}
}

/*
 * State Landed
 */
static void landed_run(void *ctx) {}

static const struct smf_state states[] = {
	[STATE_STARTUP] = SMF_CREATE_STATE(NULL, startup_run, NULL, NULL, NULL),
	[STATE_PAD_IDLE] = SMF_CREATE_STATE(pad_idle_entry, pad_idle_run, pad_idle_exit, NULL, NULL),
	[STATE_PAD_READY] = SMF_CREATE_STATE(pad_ready_entry, pad_ready_run, NULL, NULL, NULL),
	[STATE_BOOST] = SMF_CREATE_STATE(boost_entry, boost_run, boost_exit, NULL, NULL),
	[STATE_COAST] = SMF_CREATE_STATE(NULL, coast_run, NULL, NULL, NULL),
	[STATE_DESCENDING] = SMF_CREATE_STATE(NULL, descending_run, NULL, NULL, NULL),
	[STATE_LANDED] = SMF_CREATE_STATE(NULL, landed_run, NULL, NULL, NULL),
};

mission_state_t flightstate_get_state()
{
	return (mission_state_t)(smf_context.ctx.current - states);
}

bool flightstate_inflight()
{
	mission_state_t state = flightstate_get_state();
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

int flightstate_get_ignition_count() {
	return ignition_count;
}

void flightstate_set_mission() {
	mission_start_time = k_uptime_get();
}

void flightstate_init() {
	edge_detector_init(&detector_apogee, edge_detector_cond_st, 0);
	edge_detector_init(&detector_burnout, edge_detector_cond_st, 0);
	edge_detector_init(&detector_ignition, edge_detector_cond_gt, 50);
	edge_detector_init(&detector_landing, edge_detector_cond_window, 1000);
	edge_detector_init(&detector_height_liftoff, edge_detector_cond_gt, 50);

	if (signal_a_w.value.type != type_matrix) {
		LOG_ERR("expected matrix valued type for signal a_r");
	}

	if (signal_v_w.value.type != type_matrix) {
		LOG_ERR("expected matrix type for signal v_w");
	}

	if (signal_h.value.type != type_float32) {
		LOG_ERR("expected float type for signal h");
	}

	smf_set_initial(SMF_CTX(&smf_context), &states[STATE_STARTUP]);
}

void flightstate_update() {
	smf_context.t = k_uptime_get();
	smf_run_state(SMF_CTX(&smf_context));
}