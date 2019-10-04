/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/pwm.h>
#include <console/console.h>
#include <storage/flash_map.h>
#include <fs/fs.h>
#include <fs/littlefs.h>
#include <drivers/flash.h>

#include <logging/log_ctrl.h>


#define LED_PORT DT_ALIAS_LED0_GPIOS_CONTROLLER
#define LED	DT_ALIAS_LED0_GPIOS_PIN

/* 1000 msec = 1 sec */
#define SLEEP_TIME 1000


FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);

static struct fs_mount_t lfs_storage_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &storage,
	.storage_dev = (void *)DT_FLASH_AREA_STORAGE_ID,
	.mnt_point = "/lfs",
};

void err(struct device *dev){
	gpio_pin_configure(dev, LED, GPIO_DIR_OUT);
	int cnt = 0;
	
	while(1){
		cnt++;
		gpio_pin_write(dev,LED,cnt%2);
		//k_sleep(200);
	}
}

static void __attribute__((noreturn)) panic(const char *str)
{
	log_panic();
	printk("%s", str);
	k_panic();
	while(1);
}

void main(void)
{
	int cnt = 0;
	struct device *dev;
	struct device *pwmdev;
	int rc;

	printk("Starting application \n");
	//panic("Test");



	dev = device_get_binding(LED_PORT);
	if(!dev){
		panic("get binding");
	}

	rc = gpio_pin_configure(dev, LED, GPIO_DIR_OUT);
	if(rc < 0){
		panic("pin configure");
	}

	pwmdev = device_get_binding("PWM_2");
	if(pwmdev == NULL)
		panic("can not find pwm device");

    int res = pwm_pin_set_usec(pwmdev,4,500,250);
	
	if(res != 0)
		panic("unable to set pwm\n");
	

	for(int i = 0; i<3; i++){
		pwm_pin_set_usec(pwmdev, 4,500,250);
		k_sleep(900);
		pwm_pin_set_usec(pwmdev,4,500,0);
		k_sleep(30);
	}
#if 0
	struct fs_mount_t *mp = &lfs_storage_mnt;
	unsigned int id = (uintptr_t)mp->storage_dev;
	const struct flash_area *pfa;

	k_sleep(10);

	printk("Erasing flash area ... ");
	rc = flash_area_erase(pfa, 0, pfa->fa_size);
	printk("%d\n", rc);
	flash_area_close(pfa);
	goto end;

	rc = fs_mount(mp);
	if (rc < 0) {
		panic("fail mount");
	}

	// check if test file exists
	struct fs_dirent dirent;
	rc = fs_stat("/lfs/test", &dirent);
	if (rc < 0){
		panic("fs_stat");
	}
		

	// //open a new file
	// struct fs_file_t fid;
	// rc = fs_open(&fid, "/lfs/test");
	// if (rc < 0){
	// 	panic("file open");
	// }


	// char data[32] = { 0 };
	// rc = fs_read(&fid,data,dirent.size);
	// if(rc < 0){
	// 	panic("file open");
	// }

	// rc = fs_close(&fid);
	// if(rc < 0){
	// 	panic("file close");
	// }
	// char *data = "Hello World!";
	// rc = fs_write(&fid, data, 13);
	// if (rc < 0){
	// 	err(dev);
	// }

	// rc = fs_sync(&fid);
	// if (rc < 0){
	// 	panic("fs sync");
	// }


end:
#endif
	while (1) {
		/* Set pin to HIGH/LOW every 1 second */
		 gpio_pin_write(dev, LED, cnt % 2);
		cnt++;
		k_sleep(SLEEP_TIME);
	}
}
