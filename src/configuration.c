#define _DEFAULT_SOURCE 1
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
#include <errno.h>

LOG_MODULE_DECLARE(alturia);

#define xstr(s) str(s)
#define str(s) #s

ini_reader reader = (ini_reader)z_fgets;
static struct flight_config flight_cfg;
static struct sys_config sys_cfg;
static struct sys_config_valid sys_cfg_valid;
static struct flight_config_valid flight_cfg_valid;

const struct flight_config *get_flight_config()
{
	return &flight_cfg;
}

const struct sys_config *get_sys_config()
{
	return &sys_cfg;
}

static int parse_char(const char *value, char *dest, size_t dest_size)
{
	int rc = strlcpy(dest, value, dest_size);

	if (rc >= dest_size) {
		LOG_ERR("value too long");
		return 0;
	}
	return 1;
}

static int parse_int(const char *value, int *dest)
{
	errno = 0;
	int tmp = strtol(value, NULL, 10);

	if (errno != 0) {
		LOG_ERR("can not convert \"%s\" to an integer",
			log_strdup(value));
		return 0;
	}

	*dest = tmp;
	return 1;
}

static int read_sys_config_handler(void* user, const char* section,
				   const char* name, const char* value)
{
	struct sys_config *sys_cfg = (struct sys_config *)user;
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

	#define PARSE_char(n, v) parse_char(v, sys_cfg->n, \
					    ARRAY_SIZE(sys_cfg->n))
	#define PARSE_int(n, v) parse_int(v, &sys_cfg->n)
	#define PARSE_float(n, v) parse_float(v, &sys_cfg->n)

	#define FIELD(t, n, name_ext, section, a, b)\
	if (MATCH(xstr(section), xstr(n))) {\
		/* get the parse function based on type */ \
		int res = __CONCAT(PARSE_, t)(n, value);\
		if (res != 1) {\
			return res;\
		}\
		sys_cfg_valid.n = 1; \
		return 1;\
	} else
	SYS_CONFIG_SCHEMA
	{
		/* no match detected*/
		return 1;
	}
	#undef FIELD
	#undef PARSE_char
	#undef PARSE_int
	#undef PARSE_float
}

int test_sys_cfg()
{
	int res = 0;
	#define FIELD(t, n, name_ext, section, a, b) \
		if (sys_cfg_valid.n != 1) { \
			LOG_ERR("missing field %s", log_strdup(xstr(n))); \
			res = 1; \
		}
	SYS_CONFIG_SCHEMA
	#undef FIELD
	sys_cfg_valid.config_valid = true;
	return res;
}


int test_flight_cfg()
{
	int res = 0;
	#define FIELD(t, n, name_ext, section, a, b) \
		if (flight_cfg_valid.n != 1) { \
			LOG_ERR("missing field %s", log_strdup(xstr(n))); \
			res = 1; \
		}
	FLIGHT_CONFIG_SCHEMA
	#undef FIELD
	flight_cfg_valid.config_valid = true;
	return res;
}

static int read_flight_config_handler(void *user, const char *section,
				      const char *name, const char *value)
{
	struct flight_config * flight_cfg = (struct flight_config *)user;

	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

	#define PARSE_char(n, v) parse_char(v, flight_cfg->n, \
					    ARRAY_SIZE(flight_cfg->n))
	#define PARSE_int(n, v) parse_int(v, &flight_cfg->n)
	#define PARSE_float(n, v) parse_float(v, &flight_cfg->n)

	#define FIELD(t, n, name_ext, section, a, b)\
	if (MATCH(xstr(section), xstr(n))) {\
		/* get the parse function based on type */ \
		int res = __CONCAT(PARSE_, t)(n, value);\
		if (res != 1) {\
			return res;\
		}\
		flight_cfg_valid.n = 1; \
		return 1;\
	} else
	FLIGHT_CONFIG_SCHEMA
	{
		/* no match detected*/
		return 1;
	}
	#undef FIELD
	#undef PARSE_char
	#undef PARSE_int
	#undef PARSE_float
}


int read_sys_config(const char* path)
{
	int rc;
	struct fs_file_t fd;

	rc = fs_open(&fd, ALTURIA_FLASH_MP"/config/sysconfig.ini");
	if (rc != 0) {
		LOG_ERR("error opening config file: %d", rc);
		return rc;
	}

	rc = ini_parse_stream(reader, &fd, read_sys_config_handler, &sys_cfg);
	if (rc != 0) {
		rc = -EINVAL;
		goto out;
	}

	rc = test_sys_cfg();
	if (rc != 0) {
		goto out;
	}

out:
	fs_close(&fd);
	return rc;
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
	if (rc != 0) {
		rc = -EINVAL;
		goto out;
	}

	rc = test_flight_cfg();
	if (rc != 0) {
		goto out;
	}

out:
	fs_close(&fd);
	return rc;
}