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

#ifndef ALTURIA__DATALOGGER__H
#define ALTURIA__DATALOGGER__H

#include <stddef.h>
#include <stdint.h>
#include <zephyr.h>

#define DATALOG_FILE_FORMAT_VERSION 1

struct log_data {
	uint8_t id;
	size_t data_size;
	uint8_t data[];
};

/**
 * dl_open_log() - Open a new log file
 * @path: Path to the file
 *
 * Command the worker thread to open a new log file. If the file already exists,
 * the file is overriden. The function is non blocking. If the work can not be
 * queued, an negative value is returned.
 *
 * Return: error code. Negative for errors.
 */
int dl_open_log(const char *path);

/**
 * dl_close_log() - Close a open log file
 *
 * Command the worker thread to close a already open log file
 */
int dl_close_log();

/**
 * dl_alloc_track_data_buffer() - Allocate a log_data structure
 * @log_data: Pointer to a pointer of the log_data struct
 * @tid: Track id
 * @size: Size in bytes for the data member of log_data
 *
 * Allocate a log_data structure from the datalogger internal memory pool. If
 * submitted to the datalogger with dl_add_track_data() the memory is freed by
 * the datalogger working thread. If not submitted, the memory must be released
 * with k_free().
 */
int dl_alloc_track_data_buffer(struct log_data **data, uint8_t tid, size_t size);

/**
 * dl_add_track_data() - Submit log_data element to datalogger
 * @log_data: log_data allocated with dl_alloc_track_data_buffer()
 */
int dl_add_track_data(struct log_data *data);

/**
 * dl_add_track_format_chunk() - Add a track format specifier to the log file
 * @tid: Track id number
 * @format: Python struct compatible format string
 */
int dl_add_track_format_chunk(uint8_t tid, const char *format);

/**
 * dl_add_track_names_chunk() - Add track names for a track with id tid
 * @tid: Track id number
 * @names: comma seperated list of column names
 */
int dl_add_track_names_chunk(uint8_t tid, const char *names);

/**
 * dl_add_file() - Add file content to log file
 * @fid: File id number
 * @path: Path to the file which should be included
 */
int dl_add_file(uint8_t fid, const char *path);

int dl_start_track(uint8_t tid, const char *names, const char *fmt);

#endif /* ALTURIA__DATALOGGER__H */
