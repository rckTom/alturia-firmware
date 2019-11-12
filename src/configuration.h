#ifndef __ALTURIA_CONFIGURATION_H__
#define __ALTURIA_CONFIGURATION_H__

struct sys_config {
	char flightcfg_path[64];
	char owner[64];
};

struct flight_config {
	int start_delay;
};

int read_sys_config(const char* path);
int read_flight_config(const char* path);

const struct sys_config *get_sys_config();
const struct flight_config *get_flight_config();

#endif /* __ALTURIA_CONFIGURATION_H__ */