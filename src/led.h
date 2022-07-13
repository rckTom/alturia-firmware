#ifndef ALTURIA__LED__H
#define ALTURIA__LED__H

#include <stdint.h>

struct color_hsv {
	float h, s, v;
};

int led_set_color_rgb(uint32_t color);
int led_set_color_hsv(const struct color_hsv *hsv);
int led_blink(uint32_t on_time, uint32_t off_time);
int led_fade_to_hsv(const struct color_hsv *hsv, float duration);

#endif
