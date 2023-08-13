#if CONFIG_FILE_SYSTEM

#include "alturia.h"
#include "rocket/data_logging.h"
#include "signals.h"
#include "events2.h"
#include "datalogger.h"
#include <zephyr/logging/log.h>
#include "rocket/flightstate.h"
#include "edge_detector.h"
#include <zephyr/init.h>

LOG_MODULE_DECLARE(alturia);

DECLARE_SIGNAL(signal_h_raw);
DECLARE_SIGNAL(signal_v_w);
DECLARE_SIGNAL(signal_pressure);
DECLARE_SIGNAL(signal_h);

DECLARE_SIGNAL(signal_a_raw);
DECLARE_SIGNAL(signal_omega_raw);
DECLARE_SIGNAL(signal_q);

DECLARE_EVENT(event_liftoff);
DECLARE_EVENT(event_touchdown);
DECLARE_EVENT(event_pad_ready);

static char *log_name;
static bool log_active = false;
static struct log_entry_sensor_track pre_buffer[100];
static struct log_entry_sensor_track *pre_buffer_head = pre_buffer;
extern struct event2 _event2_list_start[];
extern struct event2 _event2_list_end[];

static struct log_entry_sensor_track* pre_buffer_get_next(void) {
    if (pre_buffer_head >= (pre_buffer + 100)) {
        return pre_buffer;
    }

    return ++pre_buffer_head;
}

static void callback_pad_idle(struct event2 *evt)
{
    LOG_INF("start log");
    dl_open_log(log_name);
    dl_start_track(0, "time,pressure,altitude_raw,altitude_filtered,v_w,ax,ay,az,gx,gy,gz,qw,qx,qy,qz", "qffffffffffffff");
	dl_start_track(1, "time,event_id", "qI");
    log_active=true;
    // sensor data track
}

static void stop_log(struct k_work *w) {
    LOG_INF("close log");
    dl_close_log();
}

static void callback_touchdown(struct event2 *evt)
{
    static struct k_work_delayable w;

    k_work_init_delayable(&w, stop_log);
    k_work_schedule(&w, K_SECONDS(1));
}

static void callback_event_change(struct event2 *evt)
{
    LOG_INF("event fired");
    //log every event change
    struct log_data *data;
    
    int rc = dl_alloc_track_data_buffer(&data, 1, sizeof(struct log_entry_event_track));

    if (rc != 0) {
        return;
    }

    struct log_entry_event_track *entry = (struct log_entry_event_track *) data->data;
    entry->event_id = evt - _event2_list_start;
    entry->timestamp = k_uptime_get();
    dl_add_track_data(data);
}

static int init() {
    (void) device;
    LOG_INF("init rocket data logging");
    event2_register_callback(&event_touchdown, callback_touchdown);
    event2_register_callback(&event_pad_ready, callback_pad_idle);

    for(struct event2 *iterator = _event2_list_start; iterator != _event2_list_end; iterator++) {
        event2_register_callback(iterator, callback_event_change);
    }

    return 0;
}

static void populate_log_entry(struct log_entry_sensor_track *entry) {
    entry->timestamp = k_uptime_get();
    entry->pressure = *signal_pressure.value.value_ptr.type_float32;
    entry->h_raw = *signal_h_raw.value.value_ptr.type_float32;
    entry->h_f = *signal_h.value.value_ptr.type_float32;
    entry->v_w = mat_get((*signal_v_w.value.value_ptr.type_matrix), 2, 0);
    entry->ax = mat_get((*signal_a_raw.value.value_ptr.type_matrix), 0, 0);
    entry->ay = mat_get((*signal_a_raw.value.value_ptr.type_matrix), 1, 0);
    entry->az = mat_get((*signal_a_raw.value.value_ptr.type_matrix), 2, 0);
    entry->gx = mat_get((*signal_omega_raw.value.value_ptr.type_matrix), 0, 0);
    entry->gy = mat_get((*signal_omega_raw.value.value_ptr.type_matrix), 1, 0);
    entry->gz = mat_get((*signal_omega_raw.value.value_ptr.type_matrix), 2, 0);
    entry->qw = mat_get((*signal_q.value.value_ptr.type_matrix), 0, 0);
    entry->qx = mat_get((*signal_q.value.value_ptr.type_matrix), 1, 0);
    entry->qy = mat_get((*signal_q.value.value_ptr.type_matrix), 2, 0);
    entry->qz = mat_get((*signal_q.value.value_ptr.type_matrix), 3, 0);
}

void rocket_data_logging_set_name(char *path) {
    log_name = path;
}

void rocket_data_logging(void) {
    // start logging when in flight
    // when not in flight fill pre_buffer
    // stop logging when touchdown is detected
    struct log_entry_sensor_track *dl_entry;
    struct log_data *log_item;

    if (log_active) {
        if (dl_alloc_track_data_buffer(&log_item, 0, sizeof(struct log_entry_sensor_track))!= 0) {
            LOG_ERR("unable to allocate data logger buffer");
            return;
        }
        dl_entry = (struct log_entry_sensor_track *) log_item->data;
        populate_log_entry(dl_entry);
        dl_add_track_data(log_item);
    } else {
       dl_entry = pre_buffer_get_next();
       populate_log_entry(dl_entry);
    }
}

void rocket_data_logging_stop() {
    log_active = false;
    dl_close_log();
}

SYS_INIT(init, APPLICATION, 0);
#endif
