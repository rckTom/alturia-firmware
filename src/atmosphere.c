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

#include "atmosphere.h"
#include "constants.h"
#include <math.h>

struct layer {
	float h_base, p_base, t_base, t_lapse_rate;
};

static struct layer layers[] = {
	{ -610.0f,      1.089e5f, 292.15f, -6.5e-3f },
	{ 11000.0f,             0,       0,      0.f },
	{ 20000.0f,             0,       0,  1.0e-3f },
	{ 32000.0f,             0,       0,  2.8e-3f },
	{ 47000.0f,             0,       0,      0.f },
	{ 51000.0f,             0,       0, -2.8e-3f },
	{ 71000.0f,             0,       0, -2.0e-3f }
};

struct layer *get_layer_from_h(float h)
{
	for (int i = 0; i < sizeof(layers) / sizeof(layers[0]); i++) {
		if (layers[i].h_base >= h) {
			return layers + (i - 1);
		}
	}
}

struct layer *get_layer_from_p(float p)
{
	for (int i = 0; i < sizeof(layers) / sizeof(layers[0]); i++) {
		if (layers[i].p_base <= p) {
			return layers + (i - 1);
		}
	}
}

float calc_temperature(float h)
{
	struct layer *layer = get_layer_from_h(h);

	return layer->t_base + layer->t_lapse_rate * (h - layer->h_base);
}

float calc_pressure(float h)
{
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

float calc_height(float p)
{
	struct layer *layer = get_layer_from_p(p);

	if (layer->t_lapse_rate != 0.f) {
		return layer->h_base + layer->t_base / layer->t_lapse_rate *
		       (-1.f + powf(p / layer->p_base,
				    -RS_AIR * layer->t_lapse_rate / G));
	}

	return layer->h_base - (RS_AIR * layer->t_base / G) *
	       logf(p / layer->p_base);
}

void init()
{
	for (int i = 1; i < sizeof(layers) / sizeof(layers[0]); i++) {
		layers[i].p_base = calc_pressure(layers[i].h_base);
		layers[i].t_base = calc_temperature(layers[i].h_base);
	}
}
