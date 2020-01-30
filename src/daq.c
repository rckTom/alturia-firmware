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
#include <device.h>
#include <drivers/sensor.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(DAQ, CONFIG_DAQ_LOG_LEVEL);

K_TIMER_DEFINE(sample_timer, NULL, NULL);

void daq_thread_fcn()
{
	int res;
	struct device *acc_sens =
		device_get_binding(DT_ALIAS_ACC_SENSOR_LABEL);
	struct device *press_sens =
		device_get_binding(DT_ALIAS_PRESSURE_SENSOR_LABEL);
	struct device *gyr_sens =
		device_get_binding(DT_ALIAS_GYRO_SENSOR_LABEL);

	if(acc_sens == NULL) {
		LOG_ERR("unable to get acceleration sensor");
		k_oops();
	}

	if(press_sens == NULL) {
		LOG_ERR("unable to get pressure sensor");
		k_oops();
	}

	if(gyr_sens == NULL) {
		LOG_ERR("unable to get gyro sensor");
		k_oops();
	}

	k_timer_start(&sample_timer, 0, K_MSEC(100));

	while(true) {
		k_timer_status_sync(&sample_timer);

		struct sensor_value acc_sample[3];
		struct sensor_value gyr_sample[3];
		struct sensor_value press_sample;

		s64_t time_stamp = k_uptime_get();

		res = sensor_sample_fetch(press_sens);
		if(res != 0) {
			LOG_ERR("unable to fetch values from pressure sensor");
		}

		res = sensor_sample_fetch(acc_sens);
		if(res != 0) {
			LOG_ERR("unable to fetch values from acceleration sensor");
		}

		res = sensor_sample_fetch(gyr_sens);
		if(res != 0) {
			LOG_ERR("unable to fetch values from gyro sensor");
		}

		res = sensor_channel_get(press_sens, SENSOR_CHAN_PRESS,
					 &press_sample);
		if(res != 0) {
			LOG_ERR("unable to get value pressure sensor");
		}

		res = sensor_channel_get(acc_sens, SENSOR_CHAN_ACCEL_XYZ,
					 acc_sample);
		if(res != 0) {
			LOG_ERR("unable to get value from acceleration sensor. Error code %d", res);
		}

		res = sensor_channel_get(gyr_sens, SENSOR_CHAN_GYRO_XYZ,
					 gyr_sample);
		if(res != 0) {
			LOG_ERR("unabel to get value from gyro sensor. Error code %d", res);
		}
	}
}

K_THREAD_DEFINE(daq_thread, 512,
		daq_thread_fcn, NULL, NULL, NULL,
		CONFIG_DAQ_PRIO, 0, K_NO_WAIT);

