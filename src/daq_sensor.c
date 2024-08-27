#include "daq.h"
#include <zephyr/kernel.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "alturia.h"
#include "util.h"
#include "math_ex.h"
#include <zephyr/timing/timing.h>

#define STACK_SIZE 1024

LOG_MODULE_DECLARE(DAQ, CONFIG_DAQ_LOG_LEVEL);

static K_THREAD_STACK_DEFINE(daq_sample_stack, STACK_SIZE);

K_MUTEX_DEFINE(condvar_lock);
K_MUTEX_DEFINE(new_sample_lock);

static struct k_thread daq_sample_thread;

static uint8_t decimators[DAQ_CHANNEL_END] = {1};
static struct k_timer sample_timer;
static struct k_condvar sample_timer_sync_condvar;
static struct k_condvar new_sample_condvar;
static struct k_work work;

uint32_t update_mask = 0;
static struct sensor_value press = {0};
static struct sensor_value acc[3] = {0};
static struct sensor_value gyro[3] = {0};
static struct sensor_value acc_h[2] = {0};

struct sensor_thread_data
{
    const struct device *dev;
    const char* dev_name;
    enum sensor_channel channel;
    enum daq_channel daq_channel;
    uint8_t channel_size;
    struct sensor_value *data_storage;
};

static volatile k_timeout_t new_sample_interval = K_NO_WAIT;
static volatile k_timeout_t current_sample_interval = K_NO_WAIT;

static struct sensor_thread_data sensor_thread_data[] = {
    #if DT_NODE_EXISTS(DT_ALIAS(pressure_sensor))
    {
        .dev = DEVICE_DT_GET(DT_ALIAS(pressure_sensor)),
        .dev_name = DT_NODE_FULL_NAME(DT_ALIAS(pressure_sensor)),
        .channel = SENSOR_CHAN_PRESS,
        .daq_channel = DAQ_CHANNEL_PRESSURE,
        .channel_size = 1,
        .data_storage = &press,
    },
    #endif
    #if DT_NODE_EXISTS(DT_ALIAS(acc_sensor))
    {
        .dev = DEVICE_DT_GET(DT_ALIAS(acc_sensor)),
        .dev_name = DT_NODE_FULL_NAME(DT_ALIAS(acc_sensor)),
        .channel = SENSOR_CHAN_ACCEL_XYZ,
        .daq_channel = DAQ_CHANNEL_ACC,
        .channel_size = 3,
        .data_storage = acc,
    },
    #endif
    #if DT_NODE_EXISTS(DT_ALIAS(gyro_sensor))
    {
        .dev = DEVICE_DT_GET(DT_ALIAS(gyro_sensor)),
        .dev_name = DT_NODE_FULL_NAME(DT_ALIAS(gyro_sensor)),
        .channel = SENSOR_CHAN_GYRO_XYZ,
        .daq_channel = DAQ_CHANNEL_ACC,
        .data_storage = gyro,
        .channel_size = 3,
    }
    #endif
};


static void set_sample_interval(k_timeout_t interval)
{
    int key = irq_lock();
    new_sample_interval = interval;
    irq_unlock(key);
}

static void set_decimator(enum daq_channel channel, uint8_t decimator)
{
    decimators[channel] = decimator;
}

static void sample_timer_callback(struct k_timer *timer)
{
    k_work_submit(&work);

    if (!K_TIMEOUT_EQ(new_sample_interval, K_NO_WAIT)) {
        LOG_DBG("update sample rate to %d ms", k_ticks_to_ms_ceil32(new_sample_interval.ticks));
        k_timer_start(timer, new_sample_interval, new_sample_interval);
        current_sample_interval = new_sample_interval;
        new_sample_interval = K_NO_WAIT;
    }
}

static void sample_start(struct k_work *item)
{
    ARG_UNUSED(item);
    if (k_mutex_lock(&condvar_lock, K_NO_WAIT) != 0) {
        LOG_DBG("unable to lock mutex in sample triggger callback");
        return;
    }

    k_condvar_broadcast(&sample_timer_sync_condvar);

    update_mask = 0;
    if (k_mutex_unlock(&condvar_lock) != 0) {
        LOG_DBG("unable to release mutex in sample trigger callback");
    }
}

static void multi_sensor_sample_thread(struct sensor_thread_data *data, int len)
{
    int rc = 0;

    for(int i = 0; i < len; i++) {
        struct sensor_thread_data *sensor_data = data + i;
        if (sensor_data->dev == NULL) {
            LOG_ERR("unable to get device %s. Aborting sample thread", sensor_data->dev_name);
            return;
        }

        if (!device_is_ready(sensor_data->dev)) {
            LOG_ERR("sensor device %s is not ready. Aborting sample thread", sensor_data->dev_name);
            return;
        }
    }

    while(true) {
        // wait for sample timer signal
        if (k_mutex_lock(&condvar_lock, K_FOREVER) != 0) {
            LOG_DBG("unable to block trigger mutex");
            continue;
        }

        k_condvar_wait(&sample_timer_sync_condvar, &condvar_lock, K_FOREVER);

        if (k_mutex_unlock(&condvar_lock) != 0) {
            LOG_DBG("unable to unlock trigger mutex");
            continue;
        }

        for(int i = 0; i < len; i++) {
            struct sensor_thread_data *sensor_data = data + i;
            sensor_sample_fetch(sensor_data->dev);
            sensor_channel_get(sensor_data->dev, sensor_data->channel, sensor_data->data_storage);
            update_mask |= (1 << sensor_data->daq_channel);
        }

        rc = k_mutex_lock(&new_sample_lock, K_NO_WAIT);
        if (rc != 0) {
            LOG_WRN("sample overrun");
            continue;
        }

        k_condvar_broadcast(&new_sample_condvar);

        k_mutex_unlock(&new_sample_lock);
    }    
}

static void adc_sample_thread()
{
    const struct device *dev = device_get_binding("ADC_1");
    (void) dev;
    while(true) {
        k_sleep(K_SECONDS(10));
    }
}

static int get_samples(struct daq_sample *sample)
{
    k_sched_lock();
    sample->acc_x = sensor_value_to_float(acc);
    sample->acc_y = sensor_value_to_float(acc + 1);
    sample->acc_z = sensor_value_to_float(acc + 2);
    sample->gyro_x = sensor_value_to_float(gyro);
    sample->gyro_y = sensor_value_to_float(gyro + 1);
    sample->gyro_z = sensor_value_to_float(gyro + 2);
    sample->pressure = sensor_value_to_float(&press);
    sample->acc_hg_x = sensor_value_to_float(acc_h);
    sample->acc_hg_y = sensor_value_to_float(acc_h + 1);
    k_sched_unlock();
    return 0;
}

static uint32_t get_update_mask()
{
    return update_mask;
}

static int get_state()
{
    return 1;
}

static int start()
{
    LOG_DBG("start daq_sensor");
    k_condvar_init(&new_sample_condvar);
    k_condvar_init(&sample_timer_sync_condvar);
    k_work_init(&work, sample_start);
    k_timer_init(&sample_timer, sample_timer_callback, NULL);
    k_mutex_init(&condvar_lock);
    k_mutex_init(&new_sample_lock);

    // start threads
    k_tid_t tid = k_thread_create(&daq_sample_thread, daq_sample_stack, K_THREAD_STACK_SIZEOF(daq_sample_stack),
                    (k_thread_entry_t) multi_sensor_sample_thread,
                    sensor_thread_data,
                    (void *) ARRAY_SIZE(sensor_thread_data),
                    NULL,
                    -1,
                    0,
                    K_NO_WAIT);
    k_thread_name_set(tid, "daq_sensor");

    //start sample timer
    k_timer_start(&sample_timer, K_MSEC(100), K_MSEC(0));
    return 0;
}

static int stop()
{
    k_timer_stop(&sample_timer);
    return 0;
}

static int sync()
{
    int res;
	res = k_mutex_lock(&new_sample_lock, K_FOREVER);

    if(res != 0) {
        LOG_ERR("unable to lock sync mutex");
        return res;
    }

    res = k_condvar_wait(&new_sample_condvar, &new_sample_lock, K_FOREVER);
    if(res != 0) {
        return res;
    }

    res = k_mutex_unlock(&new_sample_lock);

    return res;
}

static struct daq_api api = {
    .get_state = get_state,
    .start = start,
    .stop = stop,
    .sync = sync,
    .set_decimator = set_decimator,
    .set_sample_interval = set_sample_interval,
    .get_sample = get_samples,
    .get_update_mask = get_update_mask
};

struct daq_api *sensor_daq_get_api_provider()
{
    return &api;
}
