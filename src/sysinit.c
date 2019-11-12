#include <zephyr.h>
#include <fs/fs.h>
#include <fs/littlefs.h>
#include <storage/flash_map.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
#include "alturia.h"

#define CONFIG_APP_WIPE_STORAGE 0

LOG_MODULE_DECLARE(alturia);

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
static void make_dir_structure()
{
    int rc;
    struct fs_dirent entry;
    static const struct dirs {
        const char *path;
    } dirs[] = {
        {
            .path = ALTURIA_FLASH_MP"/config",
        },
        {
            .path = ALTURIA_FLASH_MP"/sys",
        },
        {
            .path = ALTURIA_FLASH_MP"/data",
        },
        {
            .path = ALTURIA_FLASH_MP"/user",
        }
    };

    for (int i = 0; i < ARRAY_SIZE(dirs); i++) {
        rc = fs_stat(dirs[i].path, &entry);
        if (rc == -ENOENT) {
            LOG_INF("Create directory %s", dirs[i].path);
            rc = fs_mkdir(dirs[i].path);
            if (rc != 0) {
                LOG_ERR("unable to create directory %s", dirs[i].path);
                k_oops();
            }
        } else if (rc != 0) {
            LOG_ERR("fs_stat failed");
            k_oops();
        }
    }
}

void init_fs(void)
{
	struct fs_mount_t *mp = &lfs_storage_mnt;
	unsigned int id = (uintptr_t)mp->storage_dev;
	const struct flash_area *pfa;
	int rc;

	rc = flash_area_open(id, &pfa);
	if (rc < 0) {
		LOG_ERR("Unable to find flash area %u: %d", id, rc);
		k_oops();
	}

	if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
		printk("Erasing flash area ... ");
		rc = flash_area_erase(pfa, 0, pfa->fa_size);
		printk("%d\n", rc);
		flash_area_close(pfa);
	}


	rc = fs_mount(mp);
	if (rc < 0) {
		LOG_ERR("Unable to mount id %u at %s: %d",
			(unsigned int)mp->storage_dev, mp->mnt_point, rc);
		fs_unmount(mp);
		k_oops();
	}

	make_dir_structure();

}