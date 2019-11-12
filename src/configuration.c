#include "ini.h"
#include "configuration.h"
#include "alturia.h"
#include "util.h"
#include <fs/fs.h>
#include <stdio.h>
#include <string.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <stdlib.h>

LOG_MODULE_DECLARE(alturia);

ini_reader reader = (ini_reader)z_fgets;
static struct flight_config flight_cfg;
static struct sys_config sys_cfg;

const struct flight_config *get_flight_config()
{
	return &flight_cfg;
}

const struct sys_config *get_sys_config()
{
	return &sys_cfg;
}

static int read_sys_config_handler(void* user, const char* section,
				   const char* name, const char* value)
{
	struct sys_config *sys_cfg = (struct sys_config *)user;
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

	if (MATCH("", "flightcfg_path")) {
		strlcpy(sys_cfg->flightcfg_path, value,
			ARRAY_SIZE(sys_cfg->flightcfg_path));
	} else if (MATCH("", "owner")) {
		strlcpy(sys_cfg->owner, value, ARRAY_SIZE(sys_cfg->owner));
	} else {
		return 1;
	}

	return 1;
}

static int read_flight_config_handler(void *user, const char *section,
				      const char *name, const char *value)
{
	struct flight_config * flight_cfg = (struct flight_config *)user;

	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("","start_delay")) {
		flight_cfg->start_delay = strtol(value, NULL, 10);
	}
	return 1;
}

int read_sys_config(const char* path)
{
	int rc;
	struct fs_file_t fd;

	rc = fs_open(&fd, ALTURIA_FLASH_MP"/config/sysconfig.ini");
	if (rc != 0) {
		return rc;
	}

	rc = ini_parse_stream(reader, &fd, read_sys_config_handler, &sys_cfg);

	rc = fs_close(&fd);
	if (rc != 0) {
		return rc;
	}
	LOG_INF("parse result %d", rc);
	LOG_INF("syscfg: %s", log_strdup(sys_cfg.flightcfg_path));
	LOG_INF("syscfg owner: %s", log_strdup(sys_cfg.owner));
	return 0;
}

int read_flight_config(const char *path)
{
	int rc;
	struct fs_file_t fd;

	rc = fs_open(&fd, path);
	if (rc != 0) {
		return rc;
	}

	rc = ini_parse_stream(reader, &fd, read_flight_config_handler,
			      &flight_cfg);

	LOG_INF("Start delay: %d", flight_cfg.start_delay);

	return fs_close(&fd);
}