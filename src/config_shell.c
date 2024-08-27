#include <stdlib.h>
#include <zephyr/shell/shell.h>
#include "config_yaml.h"

static int cmd_config_get(const struct shell *shell, size_t argc, char **argv, void *data)
{
	struct config *config = get_app_config();

	if (argc < 2) {
		return -1;
	}

	char *str;
	if (config_get_string(config, argv[1], &str) != 0) {
		return -1;
	}

	printk("%s\n", str);
	return 0;
}

static int cmd_config_set(const struct shell *shell, size_t argc, char **argv, void *data)
{

	struct config *config = get_app_config();

	if (argc < 3) {
		return -1;
	}

	if (config_set_string(config, argv[1], argv[2]) != 0) {
		return -1;
	}

	return 0;
}

static int cmd_config_dump(const struct shell *shell, size_t argc, char **argv, void *data)
{
	if (argc < 2) {
		return -1;
	}
	config_dump(get_app_config(), argv[1]);
	return 0;
}

static int cmd_config_show(const struct shell *shell, size_t argc, char **argv, void *data)
{
	struct config *config = get_app_config();
	const char *config_str = cJSON_Print(config->json);

	if (config_str == NULL) {
		return -1;
	}

	printk("%s\n", config_str);
	free((void *)config_str);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_config, SHELL_CMD(show, NULL, "Show config", cmd_config_show),
			       SHELL_CMD(get, NULL, "Get config parameter", cmd_config_get),
			       SHELL_CMD(set, NULL, "Set config parameter", cmd_config_set),
			       SHELL_CMD(dump, NULL, "Dump config to file", cmd_config_dump),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(config, &sub_config, "Appliation Config", NULL);
