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

#include "is_atmosphere.h"
#include "constants.h"
#include <math.h>

struct layer {
	float h_base, p_base, t_base, t_lapse_rate;
};

static struct layer layers[] = {
	{ -610.0f,      1.089e6f, 292.15f, -6.5e-3f },
	{ 11000.0f,             0,       0,      0.f },
	{ 20000.0f,             0,       0,  1.0e-3f },
	{ 32000.0f,             0,       0,  2.8e-3f },
	{ 47000.0f,             0,       0,      0.f },
	{ 51000.0f,             0,       0, -2.8e-3f },
	{ 71000.0f,             0,       0, -2.0e-3f }
};

#define LAYER_COUNT sizeof(layers) / sizeof(layers[0])

static int validate_height(float h)
{
	if (h < layers[0].h_base) {
		return -1;
	} else if (h > layers[LAYER_COUNT-1].h_base) {
		return 1;
	}
	return 0;
}

static int validate_pressure(float p)
{
	if (p > layers[0].p_base) {
		return -1;
	} else if (p < layers[LAYER_COUNT-1].p_base) {
		return 1;
	}
	return 0;
}

static struct layer *get_layer_from_h(float h)
{	int i = 1;
	for (; i < LAYER_COUNT; i++) {
		if (layers[i].h_base >= h) {
			break;
		}
	}
	return layers + (i - 1);
}

static struct layer *get_layer_from_p(float p)
{
	int i = 1;
	for (; i < LAYER_COUNT; i++) {
		if (layers[i].p_base <= p) {
			break;
		}
	}
	return layers + (i - 1);
}

float isa_calc_temperature(float h)
{
	int res = validate_height(h);

	if (res == -1) {
		return layers[0].t_base;
	} else if (res == 1) {
		return layers[LAYER_COUNT-1].t_base;
	}

	struct layer *layer = get_layer_from_h(h);

	return layer->t_base + layer->t_lapse_rate * (h - layer->h_base);
}

float isa_calc_pressure(float h)
{
	int res = validate_height(h);

	if (res == -1) {
		return layers[0].p_base;
	} else if (res == 1) {
		return layers[LAYER_COUNT-1].p_base;
	}

	struct layer *layer = get_layer_from_h(h);

	if (layer->t_lapse_rate != 0.f) {
		float base = layer->t_base / (layer->t_base +
					      layer->t_lapse_rate *
					      (h - layer->h_base));
		float exponent = G / (RS_AIR * layer->t_lapse_rate);

		return layer->p_base * powf(base, exponent);
	}

	return layer->p_base *
	       exp(-G * (h - layer->h_base) / (RS_AIR * layer->t_base));
}

float isa_calc_height(float p)
{
	int res = validate_pressure(p);

	if (res == -1) {
		return layers[0].h_base;
	} else if (res == 1) {
		return layers[LAYER_COUNT-1].h_base;
	}

	struct layer *layer = get_layer_from_p(p);

	if (layer->t_lapse_rate != 0.f) {
		return layer->h_base + layer->t_base / layer->t_lapse_rate *
		       (-1.f + powf(p / layer->p_base,
				    -RS_AIR * layer->t_lapse_rate / G));
	}

	return layer->h_base - (RS_AIR * layer->t_base / G) *
	       logf(p / layer->p_base);
}

void isa_init()
{
	for (int i = 1; i < sizeof(layers) / sizeof(layers[0]); i++) {
		layers[i].p_base = isa_calc_pressure(layers[i].h_base);
		layers[i].t_base = isa_calc_temperature(layers[i].h_base);
	}
}
