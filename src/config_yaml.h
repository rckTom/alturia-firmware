#pragma once

#include <zephyr/device.h>
#include <stdint.h>
#include "cJSON.h"

struct config {
	struct cJSON *json;
};

struct config *get_app_config();
int init_config(const struct device *dev);
int config_check(struct config *config);
int config_dump(struct config *config, const char *path);
int config_load(struct config *config, const char *path);
int config_get_string(struct config *config, const char *key, char **str);
int config_get_int(struct config *config, const char *key, uint32_t *number);
int config_set_string(struct config *config, const char *key, const char *str);
int config_set_int(struct config *config, const char *key, uint32_t number);