/*
 * Alturia Firmware - The firmware for the alturia flight computer
 *
 * Copyright (c) Thomas Schmid, 2020
 *
 * Authors:
 *  Thomas Schmid <tom@lfence.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 3.  See
 * the COPYING file in the top-level directory.
 */

#include "led.h"
#include <math.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

LOG_MODULE_REGISTER(led, CONFIG_LOG_DEFAULT_LEVEL);

K_SEM_DEFINE(led_lock, 1, 1);

static const struct device *dev = DEVICE_DT_GET(DT_ALIAS(led));

static uint32_t current_color = 0;

struct fade_work_item {
	struct k_work_delayable work;
	float t;
	float duration;
	struct color_hsv start;
	struct color_hsv end;
} fade_work;

static void fade_work_handler(struct k_work *item)
{
	struct k_work_delayable *dw = k_work_delayable_from_work(item);
	struct fade_work_item *fi =
	    CONTAINER_OF(dw, struct fade_work_item, work);

	if (fi->t >= fi->duration) {
		led_set_color_hsv(&fi->end);
		k_sem_give(&led_lock);
		return;
	}

	float hd, sd, vd;

	hd = fi->end.h - fi->start.h;
	sd = fi->end.s - fi->start.s;
	vd = fi->end.v - fi->start.v;

	struct color_hsv color = fi->start;
	float fac = fi->t / fi->duration;

	color.h += fac * hd;
	color.s += fac * sd;
	color.v += fac * vd;

	led_set_color_hsv(&color);

	fi->t += 0.05;
	k_work_reschedule(dw, K_MSEC(50));
}

static inline void rgb_to_components(uint32_t color, uint8_t *r, uint8_t *g,
				     uint8_t *b)
{
	*r = (color & 0x00FF0000) >> 16;
	*g = (color & 0x0000FF00) >> 8;
	*b = (color & 0x000000FF);
}

static void rgb_to_hsv(uint32_t rgb, struct color_hsv *hsv)
{
	float r, g, b, max, min;
	r = (float)((rgb & 0x00FF0000) >> 16) / (float)0xFF;
	g = (float)((rgb & 0x0000FF00) >> 8) / (float)0xFF;
	b = (float)(rgb & 0x000000FF) / (float)0xFF;

	max = r >= b ? r : b;
	max = max >= g ? max : g;

	min = r <= b ? r : b;
	min = min <= g ? min : g;

	if (max == min) {
		hsv->h = 0.0f;
	} else if (max == r) {
		hsv->h = 60.f * (g - b) / (max - min);
	} else if (max == g) {
		hsv->h = 60.f * (2 + (b - r) / (max - min));
	} else if (max == b) {
		hsv->h = 60.f * (4 + (r - g) / (max - min));
	}

	if (hsv->h < 0) {
		hsv->h += 360.f;
	}

	if (max == 0) {
		hsv->s = 0.f;
	} else {
		hsv->s = (max - min) / (1 - fabsf(max + min - 1));
	}
	hsv->v = max;
}

static uint32_t hsv_to_rgb(const struct color_hsv *hsv)
{
	float f, hi, p, q, t, r, g, b;
	r = 0.0f;
	g = 0.0f;
	b = 0.0f;
	f = hsv->h / 60.0;
	hi = floorf(f);
	f = f - hi;

	p = hsv->v * (1 - hsv->s);
	q = hsv->v * (1 - hsv->s * f);
	t = hsv->v * (1 - hsv->s * (1 - f));

	if (hi == 0 || hi == 6) {
		r = hsv->v;
		g = t;
		b = p;
	} else if (hi == 1) {
		r = q;
		g = hsv->v;
		b = p;
	} else if (hi == 2) {
		r = p;
		g = hsv->v;
		b = t;
	} else if (hi == 3) {
		r = p;
		g = q;
		b = hsv->v;
	} else if (hi == 4) {
		r = t;
		g = p;
		b = hsv->v;
	} else if (hi == 5) {
		r = hsv->v;
		g = p;
		b = q;
	}

	return ((uint32_t)(0xFF * r) << 16) | ((uint32_t)(0xFF * g) << 8) |
	       (uint32_t)(0xFF * b);
}

int led_fade_to_hsv(const struct color_hsv *hsv, float duration)
{
	if (k_sem_take(&led_lock, K_NO_WAIT) != 0) {
		return -EBUSY;
	}

	rgb_to_hsv(current_color, &fade_work.start);
	fade_work.end = *hsv;
	fade_work.duration = duration;
	fade_work.t = 0.0;

	k_work_init_delayable(&fade_work.work, fade_work_handler);
	k_work_schedule(&fade_work.work, K_NO_WAIT);
	return 0;
}

int led_set_color_rgb(uint32_t color)
{
	current_color = color;

	union rgb_color {
		uint32_t color;
		struct {
			uint8_t _reserved;
			uint8_t r;
			uint8_t g;
			uint8_t b;
		} rgb;
	} c;

	c.color = color;

	return led_set_color(dev, 0, 3, &c.rgb.r);
}

int led_set_color_hsv(const struct color_hsv *hsv)
{
	return led_set_color_rgb(hsv_to_rgb(hsv));
}

int init()
{
	if (!device_is_ready(dev)) {
		goto err;
	}

	return led_off(dev, 0);

err:
	LOG_ERR("unable to initiaize led subsystem");
	k_oops();
	return -1;
}

SYS_INIT(init, APPLICATION, 1);
