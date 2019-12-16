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

#include "datalogger.h"
#include <fs/fs.h>
#include <string.h>
#include <zephyr.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(datalogger, CONFIG_LOG_DEFAULT_LEVEL);

static struct fs_file_t fd;

int open_log(const char *path)
{
	return fs_open(&fd, path);
}

int close_log()
{
	return fs_close(&fd);
}

int add_track_format_chunk(uint8_t tid, const char* format)
{
	int rc;
	uint8_t bf = ((1 & 0xF) << 4) | (tid & 0xF);
	rc = fs_write(&fd, &bf, sizeof(uint8_t));
	if (rc != sizeof(uint8_t)) {
		return -EIO;
	}

	size_t l = strlen(format) + 1;
	rc = fs_write(&fd, format, l);
	if (rc != l) {
		return -EIO;
	}

	return 0;
}

int add_track_names_chunk(uint8_t tid, const char* names)
{
	int rc;
	uint8_t bf = ((2 & 0xF) << 4) | (tid & 0xF);
	rc = fs_write(&fd, &bf, sizeof(uint8_t));
	if (rc != sizeof(uint8_t)) {
		return -EIO;
	}

	size_t dl = strlen(names)+1;
	rc = fs_write(&fd, names, dl);
	if (rc != dl){
		return -EIO;
	}

	return 0;
}

int add_track_data(uint8_t tid, void *data, size_t length)
{
	int rc;
	uint8_t bf = ((3 & 0xF) << 4) | (tid & 0xF);
	rc = fs_write(&fd, &bf, sizeof(uint8_t));
	if (rc != sizeof(uint8_t)) {
		return -EIO;
	}

	rc = fs_write(&fd, data, length);
	if (rc != length){
		return -EIO;
	}

	return 0;
}

int add_file(uint8_t fid, const char *path)
{
	int rc;
	struct fs_dirent entry;
	struct fs_file_t file;
	uint8_t bf = ((4 & 0xF) << 4) | (fid & 0xF);
	u32_t size;

	rc = fs_stat(path, &entry);
	if (rc != 0) {
		return rc;
	}

	size = entry.size;
	rc = fs_write(&fd, &bf, 1);
	if (rc != 1) {
		return rc;
	}

	rc = fs_write(&fd, &size, sizeof(size));
	if (rc != sizeof(size)) {
		return rc;
	}

	rc = fs_open(&file, path);
	if (rc != 0) {
		LOG_ERR("can not open file");
		return rc;
	}

	while(1) {
		uint8_t buf;
		rc = fs_read(&file, &buf, 1);
		if (rc == 0) {
			break;
		} else if (rc < 0) {
			goto out;
		}
		printk("%c", buf);
		rc = fs_write(&fd, &buf, 1);
		if (rc != 1) {
			goto out;
		}
	}

	rc = 0;
out:
	fs_close(&file);
	return rc;
}
