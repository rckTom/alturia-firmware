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

#include "alturia.h"
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#ifdef CONFIG_FILE_SYSTEM
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#endif
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/zephyr.h>

#define CONFIG_APP_WIPE_STORAGE 0

LOG_MODULE_DECLARE(alturia);

/*
 * Pheripherals
 */

int init_usb(void)
{
	int rc = usb_enable(NULL);

	if (rc != 0) {
		LOG_ERR("unable to enable usb");
	}

	return rc;
}

int init_peripherals(const struct device *dev)
{
	int res;

	res = init_usb();
	return res;
}

#if CONFIG_FILE_SYSTEM

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
	} dirs[] = {{
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
		    }};

	for (int i = 0; i < ARRAY_SIZE(dirs); i++) {
		rc = fs_stat(dirs[i].path, &entry);
		if (rc == -ENOENT) {
			LOG_INF("Create directory %s",
				dirs[i].path);
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
#endif

SYS_INIT(init_peripherals, APPLICATION, 0);
