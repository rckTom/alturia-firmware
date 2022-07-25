#ifndef ALTURIA__CONFIG__H
#define ALTURIA__CONFIG__H

int config_read_sys_config(const char *path);
float config_get_float(const char *key);
int config_get_int(const char *key);
const char *config_get_string(const char *key);

#endif