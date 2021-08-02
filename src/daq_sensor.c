#include "daq.h"
#include "zephyr.h"
#include "alturia.h"
#include "util.h"

#define STACK_SIZE 256

static K_THREAD_STACK_DEFINE(daq_acc_stack, STACK_SIZE);
static K_THREAD_STACK_DEFINE(daq_gyro_stack, STACK_SIZE);
static K_THREAD_STACK_DEFINE(daq_press_stack, STACK_SIZE);
static K_THREAD_STACK_DEFINE(daq_adc_stack, STACK_SIZE);

K_MUTEX_DEFINE(condvar_lock);
K_MUTEX_DEFINE(sample_data_lock);

static struct k_thread daq_acc_thread;
static struct k_thread daq_gyro_thread;
static struct k_thread daq_press_thread;
static struct k_thread daq_adc_thread;

static uint8_t decimators[DAQ_CHANNEL_END] = {1};
static float samples[DAQ_CHANNEL_END] = {0.0f};
static struct k_timer sample_timer;
static struct k_condvar sample_timer_sync_condvar;
static struct k_work work;

uint32_t update_mask = 0;
static struct sensor_value press = {0};
static struct sensor_value acc[3] = {0};
static struct sensor_value gyro[3] = {0};
static struct sensor_value acc_h[2] = {0};

struct sensor_thread_data
{
    const char* dev_name;
    enum sensor_channel channel;
    enum daq_channel daq_channel;
    uint8_t channel_size;
    struct sensor_value *data_storage;
};

static volatile k_timeout_t new_sample_interval = K_NO_WAIT;
static volatile k_timeout_t current_sample_interval = K_NO_WAIT;

static struct sensor_thread_data sensor_thread_data[] = {
    {
        .dev_name = DT_LABEL(DT_ALIAS(pressure_sensor)),
        .channel = SENSOR_CHAN_PRESS,
        .daq_channel = DAQ_CHANNEL_PRESSURE,
        .channel_size = 1,
        .data_storage = &press,
    },
    {
        .dev_name = DT_LABEL(DT_ALIAS(acc_sensor)),
        .channel = SENSOR_CHAN_ACCEL_XYZ,
        .daq_channel = DAQ_CHANNEL_ACC,
        .channel_size = 3,
        .data_storage = acc,
    },
    {
        .dev_name = DT_LABEL(DT_ALIAS(gyro_sensor)),
        .channel = SENSOR_CHAN_GYRO_XYZ,
        .daq_channel = DAQ_CHANNEL_ACC,
        .data_storage = gyro,
        .channel_size = 3,
    }
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

    if (K_TIMEOUT_EQ(new_sample_interval, K_NO_WAIT)) {
        k_timer_start(timer, new_sample_interval, new_sample_interval);
        current_sample_interval = new_sample_interval;
        new_sample_interval = K_MSEC(0);
    }
}

static void sample_start(struct k_work *item)
{
    ARG_UNUSED(item);
    k_mutex_lock(&condvar_lock, K_NO_WAIT);
    update_mask = 0;
    k_condvar_broadcast(&sample_timer_sync_condvar);
    k_mutex_unlock(&condvar_lock);
}

static void sensor_sample_thread(const struct sensor_thread_data *data)
{
    const struct device *dev;
    uint8_t decimator_count = 0;
    struct sensor_value sval;

    dev = device_get_binding(data->dev_name);
    
    if (dev == NULL) {
        fatal_error();
    }

    while(true) {
        // wait for sample timer signal
        if (k_mutex_lock(&condvar_lock, K_FOREVER) != 0) {
            break;
        }

        k_condvar_wait(&sample_timer_sync_condvar, &condvar_lock, K_FOREVER);

        if (k_mutex_unlock(&condvar_lock) != 0) {
            break;
        }
        
        //skip sample if neccessary
        decimator_count++;

        if (decimator_count < decimators[data->daq_channel]) {
            continue;
        }

        decimator_count = 0;

        //read sensor data
        sensor_sample_fetch_chan(dev, data->channel);
        sensor_channel_get(dev, data->channel, data->data_storage);
        
        //mark sample as updated
        update_mask |= (1 << data->daq_channel);
    }
}

static void adc_sample_thread()
{
    const struct device *dev = device_get_binding("ADC_1");
}

static int get_samples(struct daq_sample *sample)
{
    if (k_mutex_lock(&sample_data_lock, current_sample_interval) != 0) {
        LOG_ERR("unable to obtain lock on sample data");
        return -1;
    }

    sample->acc_x = sensor_value_to_float(acc);
    sample->acc_y = sensor_value_to_float(acc + 1);
    sample->acc_z = sensor_value_to_float(acc + 2);
    sample->gyro_x = sensor_value_to_float(gyro);
    sample->gyro_y = sensor_value_to_float(gyro + 1);
    sample->gyro_z = sensor_value_to_float(gyro + 2);
    sample->pressure = sensor_value_to_float(&press);
    sample->acc_hg_x = sensor_value_to_float(acc_h);
    sample->acc_hg_y = sensor_value_to_float(acc_h + 1);

    k_mutex_unlock(&sample_unlock_data);
}

static void get_new_samples(struct daq_sample *sample)
{
    if (update_mask & DAQ_CHANNEL_ACC) {
        sample->acc_x = sensor_value_to_float(acc);
        sample->acc_y = sensor_value_to_float(acc + 1);
        sample->acc_z = sensor_value_to_float(acc + 2);
    }

    if (update_mask & DAQ_CHANNEL_PRESSURE) {
        sample->pressure = sensor_value_to_float(&press);
    }

    if (update_mask & DAQ_CHANNEL_GYR) {
        sample->gyro_x = sensor_value_to_float(gyro);
        sample->gyro_y = sensor_value_to_float(gyro + 1);
        sample->gyro_z = sensor_value_to_float(gyro + 2);
    }

    if (update_mask & DAQ_CHANNEL_ACC_HIGH) {
        sample->acc_hg_x = sensor_value_to_float(acc_h);
        sample->acc_hg_y = sensor_value_to_float(acc_h + 1);
    }
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
    k_work_init(&work, sample_start);
    k_timer_init(&sample_timer, sample_timer_callback, NULL);

    // start threads
    k_thread_create(&daq_press_thread, daq_press_stack,
                    K_THREAD_STACK_SIZEOF(daq_press_stack),
                    sensor_sample_thread,
                    sensor_thread_data, NULL, NULL,
                    -1, 0, K_NO_WAIT);
    k_thread_create(&daq_acc_thread, daq_acc_stack,
                    K_THREAD_STACK_SIZEOF(daq_acc_stack),
                    sensor_sample_thread,
                    sensor_thread_data + 1, NULL, NULL,
                    -1, 0, K_NO_WAIT);
    k_thread_create(&daq_gyro_thread, daq_gyro_stack,
                    K_THREAD_STACK_SIZEOF(daq_gyro_stack),
                    sensor_sample_thread,
                    sensor_thread_data + 2, NULL, NULL,
                    -1, 0, K_NO_WAIT);
    k_thread_create(&daq_adc_thread, daq_adc_stack,
                    K_THREAD_STACK_SIZEOF(daq_adc_stack),
                    adc_sample_thread,
                    NULL, NULL, NULL,
                    -1, 0, K_NO_WAIT);
    
    //start sample timer
    k_timer_start(&sample_timer, K_MSEC(0), K_MSEC(0));
    return 0;
}

static int stop()
{
    k_timer_stop(&sample_timer);
}

static void sync()

static struct daq_api api = {
    .get_state = get_state,
    .start = start,
    .stop = stop,
    .set_decimator = set_decimator,
    .set_sample_interval = set_sample_interval,
    .get_sample = get_samples,
    .get_new_samples = get_new_samples,
    .get_update_mask = get_update_mask
};

struct daq_api *sensor_daq_get_api_provider()
{
    return &api;
}