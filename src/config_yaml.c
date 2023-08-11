#include "cJSON.h"
#include "config_yaml.h"
#include <stdlib.h>
#include <zephyr/fs/fs.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(config, CONFIG_LOG_DEFAULT_LEVEL);

enum value_type {
	VALUE_STRING,
	VALUE_NUMBER,
	VALUE_BOOL,
};

struct default_value {
	const char *key;
	enum value_type type;
	union {
		const char *string;
		uint32_t number;
		bool boolean;
	} value;
};

static const struct default_value default_values[] = {
	{"owner", VALUE_STRING, {.string = "Thomas Schmid"}},
	{"rocket_name", VALUE_STRING, {.string = "Rocket"}},
};

struct config appconfig;

int config_load(struct config *config, const char *path)
{
	const unsigned int block_size = 254;
	int rc;
	struct fs_file_t fp;
	struct fs_dir_t fd;
	uint8_t *content = NULL;
	size_t content_allocated_size = 0;
	size_t content_length = 0;

	fs_file_t_init(&fp);
	rc = fs_open(&fp, path, (FS_O_READ));
	if (rc != 0) {
		LOG_ERR("unable to open file");
		goto out;
	}

	while (1) {
		size_t space_remaining = content_allocated_size - content_length;
		if (space_remaining == 0) {
			content = realloc(content, block_size);
			if (content == NULL) {
				rc = -ENOMEM;
				goto out_free_content;
			}

			content_allocated_size += block_size;
			space_remaining = content_allocated_size - content_length;
		}

		size_t bytes_read = fs_read(&fp, content + content_length, space_remaining);

		content_length += bytes_read;
		if (bytes_read == 0) {
			break;
		}
	}
	printk("config length: %d\n", content_length);
	printk("loaded config: %s\n", content);
	config->json = cJSON_ParseWithLength(content, content_length);

	if (config->json == NULL) {
		config->json = cJSON_CreateObject();
	}

out_free:
	rc = fs_close(&fp);
out_free_content:
	free(content);
out:
	return rc;
}

int config_dump(struct config *config, const char *path)
{
	int rc;
	struct fs_file_t fp;
	uint8_t *content = NULL;
	size_t content_length = 0;

	fs_file_t_init(&fp);
	rc = fs_open(&fp, path, (FS_O_WRITE | FS_O_CREATE));
	if (rc != 0) {
		goto out;
	}

	content = cJSON_Print(config->json);
	content_length = strlen(content);

	if (content == NULL) {
		rc = -ENOMEM;
		goto out_free;
	}

	size_t bytes_written = fs_write(&fp, content, content_length);

	if (bytes_written < content_length) {
		if ((errno != 0) | (bytes_written < 0)) {
			LOG_ERR("unable to dump config file");
			goto out_free;
		}
	}

	if (fs_truncate(&fp, bytes_written) != 0) {
		rc = -1;
		LOG_ERR("unable to truncate file");
		goto out;
	}

	rc = 0;

out_free:
	rc = fs_close(&fp);
	free(content);
out:
	return rc;
}

int config_check(struct config *config)
{
	int8_t rc = 0;

	for (unsigned int i = 0; i < ARRAY_SIZE(default_values); i++) {
		const struct default_value *v = default_values + i;

		if (cJSON_GetObjectItem(config->json, v->key) != NULL) {
			continue;
		}

		rc = 1;

		if (v->type == VALUE_STRING) {
			if (cJSON_AddStringToObject(config->json, v->key, v->value.string) ==
			    NULL) {
				goto add_error;
			}
		} else if (v->type == VALUE_BOOL) {
			if (cJSON_AddBoolToObject(config->json, v->key, v->value.boolean) == NULL) {
				goto add_error;
			}
		} else if (v->type == VALUE_NUMBER) {
			if (cJSON_AddNumberToObject(config->json, v->key, v->value.number) ==
			    NULL) {
				goto add_error;
			}
		}

		continue;
	add_error:
		LOG_ERR("unable to add default value for key %s", v->key);
		return -1;
	}

	return rc;
}

int config_set_int(struct config *config, const char *key, uint32_t number)
{
	cJSON *obj = cJSON_GetObjectItem(config->json, key);

	if (obj == NULL) {
		obj = cJSON_CreateObject();
		if (obj == NULL) {
			return -ENOMEM;
		}
	}

	cJSON_SetNumberValue(obj, (double)number);
	return 0;
}

int config_set_string(struct config *config, const char *key, const char *str)
{
	cJSON *obj = cJSON_GetObjectItem(config->json, key);

	if (obj == NULL) {
		obj = cJSON_CreateObject();
		if (obj == NULL) {
			LOG_ERR("unable to allocate memory");
			return -ENOMEM;
		}
		obj->type = cJSON_String;
	}

	if (cJSON_SetValuestring(obj, str) == NULL) {
		LOG_ERR("unable to set string");
		return -1;
	}

	return 0;
}

int config_get_int(struct config *config, const char *key, uint32_t *value)
{
	cJSON *obj = cJSON_GetObjectItem(config->json, key);

	if (obj == NULL) {
		return -ENOENT;
	}

	double dvalue = cJSON_GetNumberValue(obj);
	*value = (uint32_t)dvalue;
	return 0;
}

int config_get_string(struct config *config, const char *key, char **str)
{
	cJSON *obj = cJSON_GetObjectItem(config->json, key);

	if (obj == NULL) {
		return -ENOENT;
	}

	*str = cJSON_GetStringValue(obj);
	return 0;
}

struct config *get_app_config()
{
	return &appconfig;
}

int init_config(const struct device *dev)
{
	ARG_UNUSED(dev);

	int rc = config_load(&appconfig, "/lfs/config/env.json");
	if (rc != 0) {
		LOG_ERR("unable to load config. rc: %d", rc);
		return 0;
	}

	rc = config_check(&appconfig);
	if (rc >= 1) {
		LOG_INF("save defaults");
		config_dump(&appconfig, "/lfs/config/env.json");
	} else if (rc != 0) {
		LOG_ERR("unable to check config. rc: %d", rc);
		return -1;
	}

	return 0;
}

//SYS_INIT(init_config, APPLICATION, 0);
