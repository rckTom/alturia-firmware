#ifndef __DATALOGGER_H__
#define __DATALOGGER_H__

#include <stddef.h>

#define DATALOG_FILE_FORMAT_VERSION 1

int open_log(const char *path);
int close_log();

int add_track_data(int tid, void *data, size_t length);
int add_track_format_chunk(int tid, const char* format);
int add_track_names_chunk(int tid, const char* names);

#endif /* __DATALOGGER_H__ */