#include "servos.h"

#include <zephyr.h>
#include <stdint.h>
#include <devicetree.h>
#include <init.h>
#include <drivers/pwm.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(servos, CONFIG_LOG_DEFAULT_LEVEL);

#define SERVO_INIT_MACRO(node_id) \
	{DT_LABEL(DT_PWMS_CTLR(node_id)), \
	DT_PWMS_CHANNEL(node_id), \
	DT_PWMS_PERIOD(node_id)},

static const struct servo_config {
	const char *pwm_controller;
	int pwm_channel;
	int period;
} servo_config[] = {
	DT_FOREACH_CHILD(DT_NODELABEL(servos),SERVO_INIT_MACRO)
};

static struct servo_data_config {
	/* PWM Device */
	const struct device *pwm_dev;

	/* Maximum servo angle */
	uint32_t max_us;

	/* Minimum servo angle */
	uint32_t min_us;

	/* Current setpoint */
	float setpoint;
} servo_data[ARRAY_SIZE(servo_config)];

#define NUM_SERVOS ARRAY_SIZE(servo_config)

int servo_get_setpoint(uint8_t servo, float *setpoint)
{
	if (servo >= NUM_SERVOS) {
		return -ENODEV;
	}

	*setpoint = servo_data[servo].setpoint;

	return 0;
}

int servo_set_max_us(uint8_t servo, uint32_t max_us)
{
	if (servo >= NUM_SERVOS) {
		return -ENODEV;
	}

	servo_data[servo].max_us = max_us;
	return 0;
}

int servo_set_min_us(uint8_t servo, uint32_t min_us)
{
	if (servo >= NUM_SERVOS) {
		return -ENODEV;
	}

	servo_data[servo].min_us = min_us;
	return 0;
}

int servo_set_us(int servo, uint32_t us)
{
	struct servo_data_config *data = servo_data + servo;

	if (us > data->max_us) {
		us = data->max_us;
	} else if (us < data->min_us) {
		us = data->min_us;
	}

	data->setpoint = 2/(data->max_us-data->min_us)*us-1;

	return pwm_pin_set_usec(data->pwm_dev,
				servo_config[servo].pwm_channel,
				servo_config[servo].period/1000,
				us, 0);
}

int servo_set_angle(int servo, uint8_t angle)
{
	if (servo >= NUM_SERVOS) {
		return -ENODEV;
	};

	uint32_t pw = servo_data[servo].min_us + (servo_data[servo].max_us -
					       servo_data[servo].min_us) *
					       angle / 180;

	return servo_set_us(servo, pw);
}

static int servo_init()
{
	for(int i = 0; i < NUM_SERVOS; i++) {
		servo_data[i].pwm_dev = device_get_binding(
						servo_config[i].pwm_controller);

		if (servo_data[i].pwm_dev == NULL) {
			LOG_ERR("PWM Device not found");
			return -ENODEV;
		}

		servo_data[i].max_us = 2000;
		servo_data[i].min_us = 1000;
		servo_data[i].setpoint = 0;
	}

	return 0;
}

SYS_INIT(servo_init, APPLICATION, 0);
