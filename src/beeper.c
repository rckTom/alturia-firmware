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

#include "beeper.h"
#include <stdint.h>
#include <drivers/pwm.h>
#include <device.h>
#include <devicetree.h>
#include "alturia.h"
#include <logging/log.h>
#include <init.h>

LOG_MODULE_REGISTER(beeper, CONFIG_LOG_DEFAULT_LEVEL);
K_SEM_DEFINE(lock,1,1);

static const struct beeper_device {
	const char *pwm_controller;
	int pwm_channel;
} beeper_device = {
	DT_PWMS_LABEL_BY_IDX(DT_NODELABEL(beeper),0),
	DT_PWMS_CHANNEL_BY_IDX(DT_NODELABEL(beeper),0)
};

static const struct device *pwm_dev;

static uint8_t vol = VOLUME;

struct k_work_delayable work;

struct beep_sequenz_data {
    struct k_work_delayable work;
    uint32_t count;
    uint32_t pitch;
    int32_t delay;
} beep_sequenz;

static void beep_work_handler(struct k_work *item)
{
    int res;

    struct k_work_delayable *dw = k_work_delayable_from_work(item);
    struct beep_sequenz_data *data =
        CONTAINER_OF(dw, struct beep_sequenz_data, work);

    if (data->count % 2) {
        res = pwm_pin_set_usec(pwm_dev, beeper_device.pwm_channel, data->pitch, 0,
			       PWM_POLARITY_NORMAL);
    } else {
        res = pwm_pin_set_usec(pwm_dev, beeper_device.pwm_channel, data->pitch,
			       ((data->pitch/2)*vol)/100, PWM_POLARITY_NORMAL);
    }

    if (res){
        goto err_free_lock;
    }

    data->count--;
    if (data->count > 0) {
	k_work_reschedule(dw, K_MSEC(data->delay));
        return;
    }

err_free_lock:
    k_sem_give(&lock);
    return;
}

int beep(int32_t duration, int32_t pitch)
{
    return beepn(duration, 1, pitch);
}

static int beep_cmd(int32_t pitch)
{
	return pwm_pin_set_usec(pwm_dev, beeper_device.pwm_channel, pitch,
				((pitch/2)*vol)/100, PWM_POLARITY_NORMAL);
}

int beep_on(int32_t pitch)
{
	return beep_cmd(pitch);
}

int beep_off()
{
	return beep_cmd(0);
}


int beepn(int32_t duration, int32_t count, int32_t pitch)
{
    if (k_sem_take(&lock, K_NO_WAIT) == 0) {
        beep_sequenz.count = count*2;
        beep_sequenz.delay = duration;
        beep_sequenz.pitch = pitch;

        k_work_init_delayable(&beep_sequenz.work, beep_work_handler);
        k_work_schedule(&(beep_sequenz.work), K_NO_WAIT);
        return 0;
    }
    return -EBUSY;
}

int beeper_set_volume(uint8_t volume)
{
	if (volume > 100) {
		return -EINVAL;
	}

	vol = volume;
	return 0;
}

static int beeper_init()
{
	pwm_dev = device_get_binding(beeper_device.pwm_controller);
	if (pwm_dev == NULL) {
		LOG_ERR("unable to get device");
		return -ENODEV;
	}

	return 0;
}

SYS_INIT(beeper_init, APPLICATION, 0);
