/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/pwm.h>
#include <drivers/sensor.h>
#include <console/console.h>
#include <storage/flash_map.h>
#include <fs/fs.h>
#include <fs/littlefs.h>
#include <drivers/flash.h>
#include <string.h>
#include <stdio.h>
#include "alturia.h"

#include <logging/log_ctrl.h>


#define LED_PORT DT_ALIAS_LED0_GPIOS_CONTROLLER
#define LED	DT_ALIAS_LED0_GPIOS_PIN

/* 1000 msec = 1 sec */
#define SLEEP_TIME 1000


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
	int res;
	printk("Starting application \n");
	init_gpios();

	struct device *flash;
	flash = device_get_binding("s25fl512");
	if (!flash){
		printk("can not get flash device \n");
		return;
	}

	printk("Ereasing flash\n");
	k_sleep(1);

	/*
	res = flash_erase(flash, 
						DT_FLASH_AREA_STORAGE_OFFSET,
						DT_FLASH_AREA_STORAGE_SIZE);
	if (res < 0){
		printk("error ereasing flash %d\n", res);
		k_sleep(1);
	}
	*/

	FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
	static struct fs_mount_t mp = {
		.type = FS_LITTLEFS,
		.mnt_point = "/lfs",
		.fs_data = &storage,
		.storage_dev = DT_FLASH_AREA_STORAGE_ID,
	};

	res = fs_mount(&mp);
	if (res < 0){
		printk("error mounting filesystem");
	}

	struct fs_statvfs stat; 
	res = fs_statvfs("/lfs", &stat);
	if (res < 0){
		printk("error getting statvfs\n");
	}
	else {
		printk("bfree: %ld, blocks: %ld, bsize: %ld, rsize: %ld\n",stat.f_bfree, stat.f_blocks, stat.f_bsize, stat.f_frsize);
	}

	struct fs_dirent entry;
	res = fs_stat("/lfs/test.dat", &entry);
	if (res < 0){
		printk("fs_stat error %d \n", res);
		return;
	}

	printk("Entry name %s, is dir: %d, is file: %d\n", entry.name, entry.type == FS_DIR_ENTRY_DIR, entry.type == FS_DIR_ENTRY_FILE);

	struct fs_file_t file;

	res = fs_open(&file, "/lfs/test2.dat");
	if (res < 0){
		printk("error opening file");
		return;
	}

	const char *message = "Hello World!";
	//res = fs_write(&file, message, strlen(message));
	//res = fs_sync(&file);

	char result[32];
	res = fs_read(&file,result,strlen(message));
	if (res < 0){
		printk("error reading from file");
	}
	else{
		printk("file content %s", result);
	}


	struct device *cdc_acm;
	cdc_acm = device_get_binding("CDC_ACM_0");
	if(!cdc_acm){
		printk("can not get cdc acm device\n");
		return;
	}
	



	struct device *ms5611;

	ms5611 = device_get_binding("ms5607");
	if(!ms5611){
		printk("unable to get device ms5607");
	} else {
		while(1){
		res = sensor_sample_fetch(ms5611);
		if(res < 0){
			panic("can not fetch sensor sample");
		}
		struct sensor_value sval;
		struct sensor_value sval2; 

		res = sensor_channel_get(ms5611,SENSOR_CHAN_AMBIENT_TEMP,&sval);
		if(res < 0){
			panic("unable to get channel");
		}
		
		res = sensor_channel_get(ms5611,SENSOR_CHAN_PRESS,&sval2);
		if(res < 0){
			panic("unable to get channel");
		}
		
		//sensor_channel_get(ms5611,SENSOR_CHAN_PRESS,&sval);
		char msg[64] = {0};
		snprintf(msg, sizeof(msg), "temperature:  %f\r\npressure: %f", sensor_value_to_double(&sval),
							      sensor_value_to_double(&sval2));
		printk("%s\n",msg);
		k_sleep(1000);
		}
	}
	while (1) {
		/* Set pin to HIGH/LOW every 1 second */
		k_sleep(SLEEP_TIME);
	}
}
