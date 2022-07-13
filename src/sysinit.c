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
#include <device.h>
#include <devicetree.h>
#include <fs/fs.h>
#include <fs/littlefs.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <storage/flash_map.h>
#include <usb/usb_device.h>
#include <zephyr.h>

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

/*
 * Filesystem stuff
 */

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_storage_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &storage,
    .storage_dev = (void *)FLASH_AREA_ID(storage),
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
		LOG_ERR("Unable to mount id %u at %s: %d", id,
			mp->mnt_point, rc);
		return rc;
	}

	rc = make_dir_structure();
	if (rc < 0) {
		LOG_ERR("Unable to create default file structure");
	}

	return rc;
}

SYS_INIT(init_peripherals, APPLICATION, 0);
