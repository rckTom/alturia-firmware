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
	init_fs();
	read_sys_config(ALTURIA_FLASH_MP"/config/syscfg.ini");

	strlcat(flight_cfg_path, get_sys_config()->flightcfg_path,
		ARRAY_SIZE(flight_cfg_path));

	printk("%s\n", flight_cfg_path);
	read_flight_config(flight_cfg_path);

	struct device *ms5607 = device_get_binding("ms5607");
	if (ms5607 == NULL) {
		LOG_ERR("Error getting pressure sensor device");
		return;
	}

	struct device *bmi088_accel = device_get_binding("bmi088-accel");
	if (bmi088_accel == NULL) {
		LOG_ERR("Error getting acceleration sensor");
	}


	int i = 0;
	s64_t start;
	start_delay(get_flight_config()->start_delay);
	k_sleep(1000);
	beep_pyro_status();
	start = k_uptime_get();
	open_log(ALTURIA_FLASH_MP"/data/press_new.dat");
	add_track_format_chunk(0, "qdd");
	add_track_names_chunk(0, "time,pressure,pressure2");
	while(i < 1000)
	{
		s64_t timestamp = k_uptime_get();
		int err = sensor_sample_fetch(ms5607);
		if (err != 0) {
			LOG_ERR("unable to fetch sensor");
			return;
		}
		struct sensor_value val;
		err = sensor_channel_get(ms5607, SENSOR_CHAN_PRESS, &val);
		if (err != 0) {
			LOG_ERR("unable to get channel");
			return;
		}

		double dval = sensor_value_to_double(&val);
		union {
			u8_t data[16];
			struct{
				s64_t t;
				double pres;
				double pres2;
			} __packed val;
		} values;

		values.val.pres = dval;
		values.val.pres2 = dval*2;
		values.val.t = timestamp;

		add_track_data(0, &values, sizeof(values));

		i++;
	}
	LOG_INF("Time spent for 1000 samples %lld", k_uptime_delta(&start));
	close_log();
	while(1) {
		k_sleep(10000);
		beepn(K_MSEC(10),2,460);
	}
}
