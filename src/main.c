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

#define LED_PORT DT_ALIAS_LED0_GPIOS_CONTROLLER
#define LED	DT_ALIAS_LED0_GPIOS_PIN

/* 1000 msec = 1 sec */
#define SLEEP_TIME 1000


void main(void)
{
	printk("Starting application \n");
	int cnt = 0;
	struct device *dev;
	struct device *pwmdev;

	
	pwmdev = device_get_binding("PWM_2");
	if(pwmdev == NULL)
		printk("can not find pwm device");

        int res = pwm_pin_set_usec(pwmdev,4,500,250);
	
	if(res != 0)
		printk("unable to set pwm\n");

	
	dev = device_get_binding(LED_PORT);
	gpio_pin_configure(dev, LED, GPIO_DIR_OUT);
	k_sleep(3);
	while (1) {
		/* Set pin to HIGH/LOW every 1 second */
		gpio_pin_write(dev, LED, cnt % 2);
		cnt++;
		k_sleep(SLEEP_TIME);
	}
}
