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
#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <zephyr/init.h>

LOG_MODULE_REGISTER(pyros, CONFIG_LOG_DEFAULT_LEVEL);

#define PYRO_INIT_MACRO(node_id) \
	{.dev_name = DT_GPIO_LABEL(node_id, gpios), \
	 .pin = DT_GPIO_PIN(node_id, gpios), \
	 .flags = DT_GPIO_FLAGS(node_id, gpios)},

static struct pyro{
	const char *dev_name;
	const int pin;
	const int flags;
	const struct device *dev;
	struct k_work_delayable work;
} pyro_gpios[] = {DT_FOREACH_CHILD(DT_NODELABEL(pyros),PYRO_INIT_MACRO)};


#define NUM_PYROS ARRAY_SIZE(pyro_gpios)

void pyro_work_handler(struct k_work *work) {
	struct k_work_delayable *delayed_work = k_work_delayable_from_work(work);
	struct pyro *pyro_data = CONTAINER_OF(delayed_work, struct pyro, work);
	LOG_INF("set pyro low");
	if(gpio_pin_set(pyro_data->dev, pyro_data->pin, 0) != 0) {
		LOG_ERR("pyro pin write error. device: %s, pin: %d, flags %d",
			pyro_data->dev_name, pyro_data->pin,
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
	LOG_INF("Fire pyro %d", pyro);
	res = gpio_pin_set(pyro_gpios[pyro].dev, pyro_gpios[pyro].pin, true);
	if (res != 0) {
		LOG_ERR("unable to set pyro pin");
	}

	res = k_work_schedule(&pyro_gpios[pyro].work, K_MSEC(CONFIG_PYROS_ON_TIME));

	return res;
}

int pyros_get_fire_state(unsigned int pyro)
{
	return gpio_pin_get(pyro_gpios[pyro].dev, pyro_gpios[pyro].pin);
}

static int pyros_init()
{
	const struct device *dev;
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
				pyro_gpios[i].dev_name,
				pyro_gpios[i].pin, pyro_gpios[i].flags, res);
			return -EINVAL;
		}

		pyro_gpios[i].dev = dev;
		k_work_init_delayable(&pyro_gpios[i].work, pyro_work_handler);
	}

	return 0;
}

SYS_INIT(pyros_init, APPLICATION, 0);
