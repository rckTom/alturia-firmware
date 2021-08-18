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

#include <zephyr.h>
#include "logging/log.h"
#include "daq.h"

LOG_MODULE_REGISTER(DAQ, CONFIG_DAQ_LOG_LEVEL);

static struct daq_api *api = NULL;

int daq_set_api_provider(struct daq_api *daq)
{
	api = daq;
	return 0;
}

int daq_sync()
{
	return api->sync();
}

int daq_start()
{
	return api->start();
}

int daq_stop()
{
	return api->stop();
}

void daq_set_sample_interval(k_timeout_t interval)
{
	api->set_sample_interval(interval);
}

void daq_set_decimator(enum daq_channel channel, uint8_t decimator)
{
	api->set_decimator(channel, decimator);
}

uint32_t daq_get_update_mask()
{
	return api->get_update_mask();
}

void daq_get_sample(struct daq_sample *sample)
{
	api->get_sample(sample);
}

