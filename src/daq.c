/*
 * Alturia Firmware - The firmware for the alturia flight computer
 *
 * Copyright (c) Thomas Schmid, 2019
 *
 * Authors:
 *  Thomas Schmid <tom@lfence.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 3.  See
 * the COPYING file in the top-level directory.
 */

#include <zephyr.h>
#include <devicetree.h>
#include <device.h>
#include <drivers/sensor.h>
#include <logging/log.h>
#include "datalogger.h"
#include "is_atmosphere.h"
#include "util.h"

LOG_MODULE_REGISTER(DAQ, CONFIG_DAQ_LOG_LEVEL);

struct sensor_value acc_sample[3];
struct sensor_value gyr_sample[3];
struct sensor_value press_sample;

static const struct sensor_daq_data {
	const char *sensor_device_label;
	enum sensor_channel channel;
	struct sensor_value *sval;
} sensor_daq[] = {
	{
		.sensor_device_label = DT_LABEL(DT_ALIAS(pressure_sensor)),
		.channel = SENSOR_CHAN_PRESS,
		.sval = &press_sample
	},
	{
		.sensor_device_label = DT_LABEL(DT_ALIAS(acc_sensor)),
		.channel = SENSOR_CHAN_ACCEL_XYZ,
		.sval = acc_sample,
	},
	{
		.sensor_device_label = DT_LABEL(DT_ALIAS(gyro_sensor)),
		.channel = SENSOR_CHAN_GYRO_XYZ,
		.sval = gyr_sample,
	},
};

#define NUM_SENSOR_DAQ ARRAY_SIZE(sensor_daq)

K_SEM_DEFINE(timer_sync_sem, 0, NUM_SENSOR_DAQ);
K_SEM_DEFINE(daq_sync_sem, 0, NUM_SENSOR_DAQ);
K_MUTEX_DEFINE(data_lock);

static void timer_expr_fnc(struct k_timer *timer)
{
	for (int i = 0; i < NUM_SENSOR_DAQ; i++) {
		k_sem_give(&timer_sync_sem);
	}
}

K_TIMER_DEFINE(sample_timer, timer_expr_fnc, NULL);

static void daq_sensor_thread_fnc(void * daq_data, void *arg1, void *arg2)
{
	int res = 0;
	struct sensor_daq_data *data = daq_data;

	const struct device *dev = device_get_binding(data->sensor_device_label);

	if (dev == NULL) {
		LOG_ERR("unable to get sensor device %s",
			log_strdup(data->sensor_device_label));
		k_oops();
	}

	while (true) {
		k_sem_take(&timer_sync_sem, K_FOREVER);

		res = sensor_sample_fetch(dev);

		if (res != 0) {
			LOG_ERR("unable to fetch sensor value for device %s",
				log_strdup(data->sensor_device_label));
			k_oops();
		}

		res = sensor_channel_get(dev, data->channel, data->sval);

		if (res != 0) {
			LOG_ERR("unable to get sensor value for device %s",
				log_strdup(data->sensor_device_label));
			k_oops();
		}

		k_sem_give(&daq_sync_sem);
	}
}

int daq_start()
{
	k_timer_start(&sample_timer, K_NO_WAIT, K_MSEC(10));
	return 0;
}

void daq_stop()
{
	k_timer_stop(&sample_timer);
}

int daq_sync()
{
	int rc = 0;

	for (int i = 0; i < NUM_SENSOR_DAQ; i++) {
		rc = k_sem_take(&daq_sync_sem, K_FOREVER);
	}

	return rc;
}

K_THREAD_DEFINE(DAQ_PRES, CONFIG_SENSOR_DAQ_STACK_SIZE, daq_sensor_thread_fnc,
		&sensor_daq[0], NULL, NULL, -3, 0, 0);
K_THREAD_DEFINE(DAQ_ACC, CONFIG_SENSOR_DAQ_STACK_SIZE, daq_sensor_thread_fnc,
		&sensor_daq[1], NULL, NULL, -3, 0, 0);
K_THREAD_DEFINE(DAQ_GYR, CONFIG_SENSOR_DAQ_STACK_SIZE, daq_sensor_thread_fnc,
		&sensor_daq[2], NULL, NULL, -3, 0, 0);

