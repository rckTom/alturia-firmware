#include <zephyr.h>
#include "sysinit.h"
#include "alturia.h"
#include "beeper.h"
#include <logging/log.h>
#include <device.h>
#include <drivers/sensor.h>

LOG_MODULE_DECLARE(alturia);

void main(void)
{
	int rc;
	init_gpios();
	rc = init_fs();
	if (rc != 0) {
		LOG_ERR("unable to initialize filesystem");
	}

	struct device *ms5607 = device_get_binding("ms5607");
	if (ms5607 == NULL) {
		LOG_ERR("Error getting pressure sensor device");
		return;
	}

	struct device *bmi088_accel = device_get_binding("bmi088-accel");
	if (bmi088_accel == NULL) {
		LOG_ERR("Error getting acceleration sensor");
	}

	while(1)
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

		printk("pressure: %d.%06d\n", val.val1, val.val2);

		err = sensor_channel_get(ms5607, SENSOR_CHAN_AMBIENT_TEMP, &val);
		if (err != 0) {
			LOG_ERR("unable to get channel");
			return;
		}

		printk("temperature: %d.%06d\n", val.val1, val.val2);

		k_sleep(10000);
		beepn(K_MSEC(10),2,900);
	}
}
