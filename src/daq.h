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

#ifndef ALTURIA__DAQ__H
#define ALTURIA__DAQ__H

#include "drivers/sensor.h"

void configure_pil();

extern struct sensor_value acc_sample[3];
extern struct sensor_value gyr_sample[3];
extern struct sensor_value press_sample;

enum daq_channel {
    DAQ_CHANNEL_PRESSURE,
    DAQ_CHANNEL_ACC,
    DAQ_CHANNEL_GYR,
    DAQ_CHANNEL_ACC_HIGH,
    DAQ_CHANNEL_PYRO_VOLTAGE_1,
    DAQ_CHANNEL_PYRO_VOLTAGE_2,
    DAQ_CHANNEL_PYRO_VOLTAGE_3,
    DAQ_CHANNEL_PYRO_VOLTAGE_4,
    DAQ_CHANNEL_BAT_VOLTAGE,
    DAQ_CHANNEL_END
};

struct daq_sample {
    uint64_t time;
    float pressure;
    float acc_x, acc_y, acc_z;
    float acc_hg_x, acc_hg_y;
    float gyro_x, gyro_y, gyro_z;
    float v_pyro1, v_pyro2, v_pyro3, v_pyro4, v_bat;
};

struct daq_api {
    int (*get_state)(void);
    int (*start)(void);
    int (*stop)(void);
    void (*set_decimator)(enum daq_channel, uint8_t decimator);
    void (*set_sample_interval)(k_timeout_t interval);
    void (*get_sample)(struct daq_sample *sample);
    void (*get_new_samples)(struct daq_sample *sample);
    uint32_t (*get_update_mask)(void);
};

int daq_sync();
int daq_start();

void daq_get_sample(struct daq_sample *sample);
uint32_t daq_get_update_mask();

/* set a different daq provider */
int daq_set_api_provider(struct daq_api *daq);

#endif /* ALTURIA__DAQ__H */
