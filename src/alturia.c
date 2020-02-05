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

#include "alturia.h"
#include <logging/log_ctrl.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(alturia, CONFIG_LOG_DEFAULT_LEVEL);

void __attribute__((noreturn)) panic(const char *str)
{
	log_panic();
	printk("%s", str);
	k_panic();
	while(1);
}

static const struct {
    const char* gpio_controller;
    const u32_t gpio_pin;
    const int gpio_flags;
    const bool initial_state;
} gpios[] = {
    {
        .gpio_controller  = BARO_CS_GPIO_CONTROLLER,
        .gpio_pin = BARO_CS_GPIO_PIN,
        .gpio_flags = GPIO_OUTPUT,
        .initial_state = true,
    },
    {
        .gpio_controller  = IMU_GYRO_CS_GPIO_CONTROLLER,
        .gpio_pin = IMU_GYRO_CS_GPIO_PIN,
        .gpio_flags = GPIO_OUTPUT,
        .initial_state = true,
    },
    {
        .gpio_controller = IMU_ACC_CS_GPIO_CONTROLLER,
        .gpio_pin = IMU_ACC_CS_GPIO_PIN,
        .gpio_flags = GPIO_OUTPUT,
        .initial_state = true,
    },
    {
        .gpio_controller = ACC_CS_GPIO_CONTROLLER,
        .gpio_pin = ACC_CS_GPIO_PIN,
        .gpio_flags = GPIO_OUTPUT,
        .initial_state = true,
    },
    {
        .gpio_controller = LED_GPIO_CONTROLLER,
        .gpio_pin = LED_GPIO_PIN,
        .gpio_flags = GPIO_OUTPUT,
        .initial_state = true,
    }
};

void init_gpios(void)
{
    struct device *dev;
    uint8_t n;
    int res;

    for(n = 0; n < ARRAY_SIZE(gpios); n++){
        dev = device_get_binding(gpios[n].gpio_controller);

        if (!dev)
        {
            LOG_ERR("could not get device %s", gpios[n].gpio_controller);
            panic("could not get device\n");
        }

        res = gpio_pin_configure(dev, gpios[n].gpio_pin, gpios[n].gpio_flags);
        if (res != 0) {
            panic("could not configure pin");
        }

        if (gpios[n].gpio_flags & GPIO_INPUT) {
            continue;
        }

        res =  gpio_pin_set(dev, gpios[n].gpio_pin, gpios[n].initial_state);
        if (res != 0) {
            panic("could not set initial state\n");
        }
    }
}
