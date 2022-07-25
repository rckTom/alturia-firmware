#include "lua_execution_engine/lua_execution_engine.h"
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(alturia);

int config_read_sys_config(const char *path)
{
	char owner[32];
	if (!file_exists("/lfs/sys/config.lua")) {
		return -ENOENT;
	}

	lua_engine_dofile("/lfs/sys/config.lua");

	// TODO: Make sure config is completly read before progressing
	lua_read_global_string_field("alturia_config", "owner", owner, 32);
	return 0;
}

float config_get_float(const char *key) { return -ENOENT; }
int config_get_int(const char *key) { return -ENOENT; }
const char *config_get_string(const char *key) { return NULL; }