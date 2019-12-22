/*
 * Alturia Firmware - The firmware for the alturia flight computer
 *
 * Copyright (c) Thomas Schmid, 2019
 *
 * Authors:
 *  Thomas Schmid <tom@lfence.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 3.  See
 * the COPYING file in the top-level directory.
 */

#ifndef __DATALOGGER_H__
#define __DATALOGGER_H__

#include <stddef.h>
#include <stdint.h>
#include <zephyr.h>

#define DATALOG_FILE_FORMAT_VERSION 1

struct log_data {
	u8_t id;
	size_t data_size;
	uint8_t data[];
};

int dl_open_log(const char *path);
int dl_close_log();

int dl_alloc_track_data_buffer(struct log_data **data, uint8_t tid, size_t size);
int dl_add_track_data(struct log_data *data);
int dl_add_track_format_chunk(uint8_t tid, const char *format);
int dl_add_track_names_chunk(uint8_t tid, const char *names);
int dl_add_file(uint8_t fid, const char *path);
#endif /* __DATALOGGER_H__ */
