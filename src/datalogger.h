#ifndef __DATALOGGER_H__
#define __DATALOGGER_H__

#include <stddef.h>
#include <stdint.h>

#define DATALOG_FILE_FORMAT_VERSION 1

int open_log(const char *path);
int close_log();

int add_track_data(uint8_t tid, void *data, size_t length);
int add_track_format_chunk(uint8_t tid, const char *format);
int add_track_names_chunk(uint8_t tid, const char *names);
int add_file(uint8_t fid, const char *path);
#endif /* __DATALOGGER_H__ */