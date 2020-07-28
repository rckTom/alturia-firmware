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

#include "pyros.h"
#include "alturia.h"
#include <zephyr.h>
#include <device.h>
#include <logging/log.h>
#include <devicetree.h>
#include <init.h>

LOG_MODULE_REGISTER(pyros, CONFIG_LOG_DEFAULT_LEVEL);

#define PYRO_INIT_MACRO(node_id) \
	{.dev_name = DT_GPIO_LABEL(node_id, gpios), \
	 .pin = DT_GPIO_PIN(node_id, gpios), \
	 .flags = DT_GPIO_FLAGS(node_id, gpios)},

static struct pyro{
	const char *dev_name;
	const int pin;
	const int flags;
	struct device *dev;
	struct k_delayed_work work;
} pyro_gpios[] = {DT_FOREACH_CHILD(DT_NODELABEL(pyros),PYRO_INIT_MACRO)};


#define NUM_PYROS ARRAY_SIZE(pyro_gpios)

void pyro_work_handler(struct k_work *work) {
	struct k_delayed_work *delayed_work = CONTAINER_OF(work,
						struct k_delayed_work, work);
	struct pyro *pyro_data = CONTAINER_OF(delayed_work, struct pyro, work);
	LOG_INF("set pyro low");
	if(gpio_pin_set(pyro_data->dev, pyro_data->pin, 0) != 0) {
		LOG_ERR("pyro pin write error. device: %s, pin: %d, flags %d",
			log_strdup(pyro_data->dev_name), pyro_data->pin,
			pyro_data->flags);
	}
}

int pyros_fire(unsigned int pyro)
{
	int res;

	if (pyro >= NUM_PYROS) {
		LOG_ERR("No pyro with index %d available. Can not fire", pyro);
		return -ENODEV;
	}
	res = gpio_pin_set(pyro_gpios[pyro].dev, pyro_gpios[pyro].pin, true);
	if (res != 0) {
		LOG_ERR("unable to set pyro pin");
	}

	res = k_delayed_work_submit(&pyro_gpios[pyro].work,
				    K_MSEC(CONFIG_PYROS_ON_TIME));

	return res;
}

static int pyros_init()
{
	struct device *dev;
	int res;

	for (int i = 0; i < NUM_PYROS; i++) {
		dev = device_get_binding(pyro_gpios[i].dev_name);
		if (dev == NULL) {
			LOG_ERR("Device not found");
			return -ENODEV;
		}

		res = gpio_pin_configure(dev, pyro_gpios[i].pin,
					 GPIO_OUTPUT_LOW);

		if (res != 0) {
			LOG_ERR("Unable to initialize pyro on %s pin %d with"
			        "flags %d. Error code %d",
				log_strdup(pyro_gpios[i].dev_name),
				pyro_gpios[i].pin, pyro_gpios[i].flags, res);
			return -EINVAL;
		}

		pyro_gpios[i].dev = dev;
		k_delayed_work_init(&pyro_gpios[i].work, pyro_work_handler);
	}

	return 0;
}

SYS_INIT(pyros_init, APPLICATION, 0);
