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
#include <fs/fs.h>
#include <fs/littlefs.h>
#include <storage/flash_map.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <usb/usb_device.h>
#include "alturia.h"

#define CONFIG_APP_WIPE_STORAGE 0

LOG_MODULE_DECLARE(alturia);

/*
 * Pheripherals
 */

static const struct {
	const char* gpio_controller;
	const u32_t gpio_pin;
	const int gpio_flags;
	const bool initial_state;
} gpios[] = {
	{
		.gpio_controller = DT_ALIAS_PRESSURE_SENSOR_CS_GPIOS_CONTROLLER,
		.gpio_pin = DT_ALIAS_PRESSURE_SENSOR_CS_GPIOS_PIN,
		.gpio_flags = GPIO_OUTPUT,
		.initial_state = true,
	},
	{
		.gpio_controller = DT_ALIAS_GYRO_SENSOR_CS_GPIOS_CONTROLLER,
		.gpio_pin = DT_ALIAS_GYRO_SENSOR_CS_GPIOS_PIN,
		.gpio_flags = GPIO_OUTPUT,
		.initial_state = true,
	},
	{
		.gpio_controller = DT_ALIAS_ACC_SENSOR_CS_GPIOS_CONTROLLER,
		.gpio_pin = DT_ALIAS_ACC_SENSOR_CS_GPIOS_PIN,
		.gpio_flags = GPIO_OUTPUT,
		.initial_state = true,
	},
	{
		.gpio_controller = DT_ALIAS_HIGHG_SENSOR_CS_GPIOS_CONTROLLER,
		.gpio_pin = DT_ALIAS_HIGHG_SENSOR_CS_GPIOS_PIN,
		.gpio_flags = GPIO_OUTPUT,
		.initial_state = true,
	},
	{
		.gpio_controller = DT_ALIAS_LED0_GPIOS_CONTROLLER,
		.gpio_pin = DT_ALIAS_LED0_GPIOS_PIN,
		.gpio_flags = GPIO_OUTPUT,
		.initial_state = true,
	}
};

static int init_gpios(void)
{
    struct device *dev;
    uint8_t n;
    int res;

	for(n = 0; n < ARRAY_SIZE(gpios); n++){
		dev = device_get_binding(gpios[n].gpio_controller);

		if (!dev)
		{
			LOG_ERR("could not get device %s",
				gpios[n].gpio_controller);
			panic();
		}

		res = gpio_pin_configure(dev, gpios[n].gpio_pin,
					 gpios[n].gpio_flags);
		if (res != 0) {
			LOG_ERR("could not configure pin");
			panic();
		}

		if (gpios[n].gpio_flags & GPIO_INPUT) {
			continue;
		}

		res =  gpio_pin_set(dev, gpios[n].gpio_pin,
				    gpios[n].initial_state);
		if (res != 0) {
			LOG_ERR("could not set initial state");
			panic();
		}
	}

	return res;
}

static int init_usb(void)
{
	int rc = usb_enable(NULL);

	if (rc != 0) {
		LOG_ERR("unable to enable usb");
	}

	return rc;
}

int init_peripherals(void)
{
	int res;

	res = init_gpios();
	if (res != 0) {
		return res;
	}

	res = init_usb();
	return res;
}

/*
* Filesystem stuff
*/

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_storage_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &storage,
	.storage_dev = (void *)DT_FLASH_AREA_STORAGE_ID,
	.mnt_point = ALTURIA_FLASH_MP,
};

/**
 * Check directory structure and create appropriate structure if it does not
 * exist
 **/
static int make_dir_structure()
{
	int rc;
	struct fs_dirent entry;
	static const struct dirs {
	const char *path;
	} dirs[] = {
		{
			.path = ALTURIA_FLASH_MP "/config",
		},
		{
			.path = ALTURIA_FLASH_MP "/sys",
		},
		{
			.path = ALTURIA_FLASH_MP "/data",
		},
		{
			.path = ALTURIA_FLASH_MP "/user",
		}
	};

	for (int i = 0; i < ARRAY_SIZE(dirs); i++) {
		rc = fs_stat(dirs[i].path, &entry);
		if (rc == -ENOENT) {
			LOG_INF("Create directory %s", dirs[i].path);
			rc = fs_mkdir(dirs[i].path);
			if (rc != 0) {
				LOG_ERR("unable to create directory %s",
					dirs[i].path);
				break;
			}
		} else if (rc != 0) {
			LOG_ERR("fs_stat failed");
			break;
		}
	}

	return rc;
}

int init_fs(void)
{
	struct fs_mount_t *mp = &lfs_storage_mnt;
	unsigned int id = (uintptr_t)mp->storage_dev;
	const struct flash_area *pfa;
	int rc;

	rc = flash_area_open(id, &pfa);
	if (rc < 0) {
		LOG_ERR("Unable to find flash area %u: %d", id, rc);
		return rc;
	}

	if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
		LOG_INF("Erasing flash area ... ");
		rc = flash_area_erase(pfa, 0, pfa->fa_size);
		LOG_INF("Done");
		flash_area_close(pfa);
	}

	rc = fs_mount(mp);
	if (rc < 0) {
		LOG_ERR("Unable to mount id %u at %s: %d",
			(unsigned int)mp->storage_dev, mp->mnt_point, rc);
		fs_unmount(mp);
		return rc;
	}

	rc = make_dir_structure();
	if (rc < 0) {
		LOG_ERR("Unable to create default file structure");
	}

	return rc;
}
