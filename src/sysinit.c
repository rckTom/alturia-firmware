#include <zephyr.h>
#include <fs/fs.h>
#include <fs/littlefs.h>
#include <storage/flash_map.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>

#define CONFIG_APP_WIPE_STORAGE 1

LOG_MODULE_DECLARE(alturia);

/*
* Filesystem stuff
*/

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_storage_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &storage,
	.storage_dev = (void *)DT_FLASH_AREA_STORAGE_ID,
	.mnt_point = "/lfs",
};

int init_fs(void)
{
    struct fs_mount_t *mp = &lfs_storage_mnt;
    unsigned int id = (uintptr_t)mp->storage_dev;
    int rc;

    const struct flash_area *pfa;
    rc = flash_area_open(id, &pfa);
    if (rc < 0) {
        LOG_ERR("Unable to find flash area %u: %d", id, rc);
        return -ENODEV;
    }

	if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
		printk("Erasing flash area ... ");
		rc = flash_area_erase(pfa, 0, pfa->fa_size);
		printk("%d\n", rc);
		flash_area_close(pfa);
	}


    rc = fs_mount(mp);
    if (rc < 0) {
        LOG_ERR("Unable to mount id %u at %s: %d", (unsigned int)mp->storage_dev,
                    mp->mnt_point, rc);
        fs_unmount(mp);
        return rc;
    }

    return 0;
}