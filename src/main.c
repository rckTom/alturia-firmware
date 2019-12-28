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

#define _DEFAULT_SOURCE 1
#include <zephyr.h>
#include "sysinit.h"
#include "alturia.h"
#include "beeper.h"
#include <logging/log.h>
#include <device.h>
#include <drivers/sensor.h>
#include "configuration.h"
#include <string.h>
#include <fs/fs.h>
#include "datalogger.h"
#include "arm_math.h"
#include "usb/usb_device.h"

LOG_MODULE_DECLARE(alturia);

static void start_delay(int delay) {
	while (delay-- > 0) {
		if (delay >= 3) {
			k_sleep(K_SECONDS(1));
			beep(50,400);
		}
		else if (delay >= 2) {
			k_sleep(K_MSEC(500));
			beep(50,400);
			k_sleep(K_MSEC(500));
			beep(50,400);
		} else {
			k_sleep(K_MSEC(250));
			beep(50,400);
			k_sleep(K_MSEC(250));
			beep(50,450);
			k_sleep(K_MSEC(250));
			beep(50,400);
			k_sleep(K_MSEC(250));
			beep(50,450);
		}
	}
}


void beep_pyro_status()
{
	for (int i = 0; i<4; i++) {
		if (i % 2 == 0) {
			beepn(80,2,400);
			k_sleep(1000);
		} else {
			beep(250,450);
			k_sleep(1000);
		}
	}
}
void main(void)
{
	int rc;
	char flight_cfg_path[32] = ALTURIA_FLASH_MP;
	init_gpios();

	rc = usb_enable(NULL);
	if (rc != 0) {
		LOG_ERR("unable to enable usb");
		return;
	}

	init_fs();

	rc = read_sys_config(ALTURIA_FLASH_MP"/config/syscfg.ini");
	if (rc != 0) {
		LOG_ERR("can not read systemconfig. Aborting");
		return;
	}

	set_volume(get_sys_config()->beeper_volume);
	strlcat(flight_cfg_path, get_sys_config()->flightcfg_path,
		ARRAY_SIZE(flight_cfg_path));

	LOG_DBG("%s\n", log_strdup(flight_cfg_path));
	rc = read_flight_config(flight_cfg_path);
	if (rc != 0) {
		LOG_ERR("unable to read flightcfg. Aborting");
		return;
	}


	struct device *ms5607 = device_get_binding("ms5607");
	if (ms5607 == NULL) {
		LOG_ERR("Error getting pressure sensor device");
		k_oops();
	}

	struct device *bmi088_accel = device_get_binding("bmi088-accel");
	if (bmi088_accel == NULL) {
		LOG_ERR("Error getting acceleration sensor");
		k_oops();
	}

	struct device *bmi088_gyro = device_get_binding("bmi088-gyro");
	if (bmi088_gyro == NULL) {
		LOG_ERR("Error getting gyro sensor");
		k_oops();
	}

	int i = 0;
	s64_t start;
	start_delay(get_flight_config()->start_delay);
	k_sleep(1000);
	beep_pyro_status();
	start = k_uptime_get();

	rc = dl_open_log(ALTURIA_FLASH_MP"/data/press_new.dat");
	if (rc != 0) {
		LOG_ERR("unable to open log file");
		return;
	}

	rc = dl_add_track_format_chunk(0, "qdddddddd");
	if(rc != 0) {
		LOG_ERR("unable to write format string");
		return;
	}

	rc = dl_add_track_names_chunk(0, "time,pressure,pressure2,accx,accy,accz,gyrx,gyry,gyrz");
	if(rc != 0) {
		LOG_ERR("unable to write track names");
		return;
	}

	while(i < 1000)
	{
		s64_t timestamp = k_uptime_get();
		int err = sensor_sample_fetch(ms5607);
		if (err != 0) {
			LOG_ERR("unable to fetch sensor");
			return;
		}

		err = sensor_sample_fetch_chan(bmi088_accel, SENSOR_CHAN_ACCEL_XYZ);
		if (err != 0) {
			LOG_ERR("unable to fetch sensor");
			return;
		}

		err = sensor_sample_fetch_chan(bmi088_gyro, SENSOR_CHAN_GYRO_XYZ);
		if (err != 0) {
			LOG_ERR("unable to fetch gyro");
			return;
		}

		struct sensor_value val;
		struct sensor_value acc[3];
		struct sensor_value gyr[3];
		err = sensor_channel_get(ms5607, SENSOR_CHAN_PRESS, &val);
		if (err != 0) {
			LOG_ERR("unable to get channel");
			return;
		}

		err = sensor_channel_get(bmi088_accel, SENSOR_CHAN_ACCEL_X,
					 acc);
		err = sensor_channel_get(bmi088_accel, SENSOR_CHAN_ACCEL_Y,
					 acc+1);
		err = sensor_channel_get(bmi088_accel, SENSOR_CHAN_ACCEL_Z,
					 acc+2);

		err = sensor_channel_get(bmi088_gyro, SENSOR_CHAN_GYRO_X, gyr);
		err = sensor_channel_get(bmi088_gyro, SENSOR_CHAN_GYRO_Y, gyr+1);
		err = sensor_channel_get(bmi088_gyro, SENSOR_CHAN_GYRO_Z, gyr+2);

		double dval = sensor_value_to_double(&val);
		union values{
			u8_t data[16];
			struct{
				s64_t t;
				double pres;
				double pres2;
				double acc_x;
				double acc_y;
				double acc_z;
				double gyr_x;
				double gyr_y;
				double gyr_z;
			} __packed val;
		};
		struct log_data *logdata;
		dl_alloc_track_data_buffer(&logdata, 0, sizeof(union values));

		union values *values = (union values *)logdata->data;
		values->val.pres = dval;
		values->val.pres2 = dval*2;
		values->val.acc_x = sensor_value_to_double(acc);
		values->val.acc_y = sensor_value_to_double(acc+1);
		values->val.acc_z = sensor_value_to_double(acc+2);
		values->val.gyr_x = sensor_value_to_double(gyr);
		values->val.gyr_y = sensor_value_to_double(gyr+1);
		values->val.gyr_z = sensor_value_to_double(gyr+2);
		values->val.t = timestamp;

		rc = dl_add_track_data(logdata);
		if (rc != 0) {
			LOG_ERR("can not write data");
			break;
		}
		i++;
	}
	LOG_INF("Time spent for 1000 samples %lld", k_uptime_delta(&start));
	dl_close_log();
	while(1) {
		k_sleep(10000);
		beepn(K_MSEC(10),1,400);
		struct sensor_value sval;
		sensor_sample_fetch(bmi088_accel);
		sensor_channel_get(bmi088_accel, SENSOR_CHAN_ACCEL_X, &sval);
		LOG_INF("x acceleration %d.%06d", sval.val1, sval.val2);
	}
}
