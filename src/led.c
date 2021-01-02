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
#include <device.h>
#include <devicetree.h>
#include <drivers/pwm.h>
#include <init.h>
#include <logging/log.h>
#include <math.h>
#include <zephyr.h>

LOG_MODULE_REGISTER(led, CONFIG_LOG_DEFAULT_LEVEL);

#define PERIOD 1e6 /* Period in nano seconds */

K_SEM_DEFINE(led_lock, 1, 1);

static const struct device *led_red, *led_green, *led_blue;
static uint32_t current_color = 0;

struct fade_work_item {
	struct k_delayed_work work;
	float t;
	float duration;
	struct color_hsv start;
	struct color_hsv end;
} fade_work;

static void fade_work_handler(struct k_work *item)
{
	struct k_delayed_work *dw =
	    CONTAINER_OF(item, struct k_delayed_work, work);
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
	k_delayed_work_submit(dw, K_MSEC(50));
}

static inline void rgb_to_components(uint32_t color, uint8_t *r, uint8_t *g,
				     uint8_t *b)
{
	*r = (color & 0x00FF0000) >> 16;
	*g = (color & 0x0000FF00) >> 8;
	*b = (color & 0x000000FF);
}

static inline void rgb_to_pwm(uint32_t color, uint32_t *r, uint32_t *g,
			      uint32_t *b)
{
	*r = ((color & 0x00FF0000) >> 16) * PERIOD / 0xFF;
	*g = ((color & 0x0000FF00) >> 8) * PERIOD / 0xFF;
	*b = (color & 0x000000FF) * PERIOD / 0xFF;
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

	k_delayed_work_init(&fade_work.work, fade_work_handler);
	k_work_submit(&fade_work.work.work);
	return 0;
}

int led_set_color_rgb(uint32_t color)
{
	int ret;
	uint32_t r, g, b;
	current_color = color;

	rgb_to_pwm(color, &r, &g, &b);

	ret = pwm_pin_set_nsec(led_red, DT_PWMS_CHANNEL(DT_ALIAS(red_led)), 1e6,
			       r, DT_PWMS_FLAGS(DT_ALIAS(red_led)));

	if (ret != 0) {
		goto err;
	}
	ret = pwm_pin_set_nsec(led_green, DT_PWMS_CHANNEL(DT_ALIAS(green_led)),
			       1e6, g, DT_PWMS_FLAGS(DT_ALIAS(green_led)));

	if (ret != 0) {
		goto err;
	}
	ret = pwm_pin_set_nsec(led_blue, DT_PWMS_CHANNEL(DT_ALIAS(blue_led)),
			       1e6, b, DT_PWMS_FLAGS(DT_ALIAS(blue_led)));

	if (ret != 0) {
		goto err;
	}

	return 0;
err:
	LOG_ERR("unable to set rgb led color");
	return ret;
}

int led_set_color_hsv(const struct color_hsv *hsv)
{
	return led_set_color_rgb(hsv_to_rgb(hsv));
}

int init(const struct device *dev)
{
	int ret;

	led_red = device_get_binding(DT_PWMS_LABEL(DT_ALIAS(red_led)));
	led_green = device_get_binding(DT_PWMS_LABEL(DT_ALIAS(green_led)));
	led_blue = device_get_binding(DT_PWMS_LABEL(DT_ALIAS(blue_led)));

	if (!led_red || !led_green || !led_blue) {
		goto err;
	}

	ret = pwm_pin_set_usec(led_red, DT_PWMS_CHANNEL(DT_ALIAS(red_led)),
			       1000, 0, DT_PWMS_FLAGS(DT_ALIAS(red_led)));

	if (ret != 0) {
		goto err;
	}

	ret = pwm_pin_set_nsec(led_green, DT_PWMS_CHANNEL(DT_ALIAS(green_led)),
			       1000000, 0, DT_PWMS_FLAGS(DT_ALIAS(green_led)));
	if (ret != 0) {
		goto err;
	}

	ret = pwm_pin_set_usec(led_blue, DT_PWMS_CHANNEL(DT_ALIAS(blue_led)),
			       1000, 0, DT_PWMS_FLAGS(DT_ALIAS(blue_led)));
	if (ret != 0) {
		goto err;
	}

	struct color_hsv hsv = {
	    .h = 290,
	    .s = 1,
	    .v = 1,
	};

	return 0;

err:
	LOG_ERR("unable to initiaize led subsystem");
	k_oops();
	return -1;
}

SYS_INIT(init, APPLICATION, 1);
