#include <zephyr.h>
#include "sysinit.h"
#include "alturia.h"
#include "beeper.h"
#include <logging/log.h>
#include <device.h>
#include <drivers/sensor.h>
#include "configuration.h"
#include <fs/fs.h>

LOG_MODULE_DECLARE(alturia);

static void start_delay(int delay) {
	while (delay-- > 0) {
		if (delay >= 3) {
			k_sleep(K_SECONDS(1));
			beep(50,450);
		}
		else if (delay >= 2) {
			k_sleep(K_MSEC(500));
			beep(50,450);
			k_sleep(K_MSEC(500));
			beep(50,450);
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

void main(void)
{
	int rc;
	init_gpios();
	init_fs();
	read_sys_config(ALTURIA_FLASH_MP"/config/syscfg.ini");
	read_flight_config(ALTURIA_FLASH_MP"/config/defaultfc.ini");

	struct device *ms5607 = device_get_binding("ms5607");
	if (ms5607 == NULL) {
		LOG_ERR("Error getting pressure sensor device");
		return;
	}

	struct device *bmi088_accel = device_get_binding("bmi088-accel");
	if (bmi088_accel == NULL) {
		LOG_ERR("Error getting acceleration sensor");
	}

	struct fs_file_t fd;
	rc = fs_open(&fd, "/lfs/data/press.dat");
	if(rc < 0){
		LOG_ERR("unable top open file");
	}
	int i = 0;
	s64_t start, end;
	start_delay(get_flight_config()->start_delay);
	start = k_uptime_get();
	while(i < 1000)
	{
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
		rc = fs_write(&fd, &dval, sizeof(double));
		if(rc < 8) {
			LOG_ERR("write error: %d", rc);
			break;
		}
		i++;
	}
	LOG_INF("Time spent for 1000 samples %d", k_uptime_delta(&start));
	fs_close(&fd);
	while(1) {
		k_sleep(10000);
		beepn(K_MSEC(10),2,900);
	}
}
