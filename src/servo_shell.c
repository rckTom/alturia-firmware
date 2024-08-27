#include <stdlib.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include "servos.h"

LOG_MODULE_REGISTER(servo_shell, CONFIG_LOG_DEFAULT_LEVEL);

static int cmd_servo_set(const struct shell *shell, size_t argc, char **argv, void *data)
{
    int servo_number;
    int pos;

	if (argc < 3) {
		return -1;
	}

	servo_number = atoi(argv[1]);
    pos = atoi(argv[2]);

    if (servo_set_angle(servo_number, pos) != 0) {
		LOG_ERR("unable to set servo position");
		return -1;
	}

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_servo, SHELL_CMD(set, NULL, "set servo position", cmd_servo_set),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(servo, &sub_servo, "Servo controll", NULL);
