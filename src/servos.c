#include "servos.h"

#include <zephyr.h>
#include <devicetree.h>
#include <init.h>
#include <drivers/pwm.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(servos, CONFIG_LOG_DEFAULT_LEVEL);

#define SERVO_INIT_MACRO(node_id) \
	{DT_PWMS_LABEL(node_id), \
	DT_PWMS_CHANNEL(node_id), \
	DT_PWMS_CELL_BY_IDX(node_id, period, 0)},

static const struct servo_config {
	const char *pwm_controller;
	int pwm_channel;
	int period;
} servo_config[] = {
	DT_FOREACH_CHILD(DT_NODELABEL(servos),SERVO_INIT_MACRO)
};

static struct servo_data_config {
	/* PWM Device */
	struct device *pwm_dev;

	/* Maximum servo angle */
	u32_t max_us;

	/* Minimum servo angle */
	u32_t min_us;
} servo_data[ARRAY_SIZE(servo_config)];

#define NUM_SERVOS ARRAY_SIZE(servo_config)

int servo_set_max_us(u8_t servo, u32_t max_us)
{
	if (servo >= NUM_SERVOS) {
		return -ENODEV;
	}

	servo_data[servo].max_us = max_us;
	return 0;
}

int servo_set_min_us(u8_t servo, u32_t min_us)
{
	if (servo >= NUM_SERVOS) {
		return -ENODEV;
	}

	servo_data[servo].min_us = min_us;
	return 0;
}

int servo_set_us(int servo, u32_t us)
{
	const struct servo_data_config *data = servo_data + servo;

	if (us > data->max_us) {
		us =data->max_us;
	} else if (us < data->min_us) {
		us = data->min_us;
	}

	return pwm_pin_set_usec(data->pwm_dev,
				servo_config[servo].pwm_channel,
				servo_config[servo].period/1000,
				us, 0);
}

int servo_set_angle(int servo, u8_t angle)
{
	if (servo >= NUM_SERVOS) {
		return -ENODEV;
	};

	u32_t pw = servo_data[servo].min_us + (servo_data[servo].max_us -
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
	}

	return 0;
}

SYS_INIT(servo_init, APPLICATION, 0);
