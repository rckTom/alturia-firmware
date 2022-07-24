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
#include <zephyr/devicetree.h>
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <zephyr/zephyr.h>

#ifdef CONFIG_FILE_SYSTEM

LOG_MODULE_REGISTER(datalogger, CONFIG_DATALOGGER_LOG_LEVEL);
K_HEAP_DEFINE(mem_pool, CONFIG_DATALOGGER_BUFFER_SIZE);
K_FIFO_DEFINE(fifo);

static struct fs_file_t fd;
static bool log_open;
static char current_path[32];

enum consumer_cmd {
	LOG_TRACK_FORMAT,
	LOG_TRACK_DATA,
	LOG_TRACK_NAMES,
	LOG_FILE,
	OPEN_LOG,
	CLOSE_LOG,
};

struct fifo_item {
	void *fifo_reserved;
	enum consumer_cmd cmd;
	bool free_data;
	struct log_data *data;
};

#define FIFO_ITEM_SIZE sizeof(struct fifo_item)

#define INIT_FIFO_ITEM(item, command, data_ptr, freeData) { \
	item->cmd = command;\
	item->data = data_ptr;\
	item->free_data = freeData; }\

int dl_open_log(const char *path)
{
	struct fifo_item *item;

	item = k_heap_alloc(&mem_pool, FIFO_ITEM_SIZE, K_NO_WAIT);
	if (item == NULL) {
		return -ENOMEM;
	}

	INIT_FIFO_ITEM(item, OPEN_LOG, NULL, true);

	strncpy(current_path, path, ARRAY_SIZE(current_path));
	k_fifo_put(&fifo, item);
	return 0;
}

int dl_close_log()
{
	struct fifo_item *item;

	item = k_heap_alloc(&mem_pool, FIFO_ITEM_SIZE, K_NO_WAIT);
	if (item == NULL) {
		return -ENOMEM;
	}

	INIT_FIFO_ITEM(item, CLOSE_LOG, NULL, true);
	item->cmd = CLOSE_LOG;
	k_fifo_put(&fifo, item);
	return 0;
}

int dl_add_file(uint8_t fid, const char *path)
{
	struct fifo_item *item;
	struct log_data *file_data;

	item = k_heap_alloc(&mem_pool, FIFO_ITEM_SIZE, K_NO_WAIT);
	if (item == NULL) {
		return -ENOMEM;
	}

	file_data = k_heap_alloc(&mem_pool, sizeof(struct log_data) +
			         strlen(path) + 1, K_NO_WAIT);

	if (file_data == NULL) {
		return -ENOMEM;
	}


	file_data->id = fid;
	file_data->data_size = strlen(path) + 1;
	strncpy(file_data->data, path, file_data->data_size);

	INIT_FIFO_ITEM(item, LOG_FILE, file_data, true);

	k_fifo_put(&fifo, item);
	return 0;
}

int dl_add_track_format_chunk(uint8_t tid, const char *format)
{
	struct fifo_item *item;

	item = k_heap_alloc(&mem_pool, FIFO_ITEM_SIZE, K_NO_WAIT);
	if (item == NULL) {
		return -ENOMEM;
	}

	INIT_FIFO_ITEM(item, LOG_TRACK_FORMAT, NULL, true);
	
	item->data = k_heap_alloc(&mem_pool, sizeof(struct log_data) +
				  strlen(format) + 1, K_NO_WAIT);

	if (item->data == NULL) {
		return -ENOMEM;
	}

	item->data->data_size = strlen(format) + 1;
	strncpy(item->data->data, format, item->data->data_size);
	item->data->id = tid;

	k_fifo_put(&fifo, item);
	return 0;
}

int dl_alloc_track_data_buffer(struct log_data **data, uint8_t tid, size_t size)
{
	*data = k_heap_alloc(&mem_pool, sizeof(struct log_data) + size, K_NO_WAIT);

	if (*data == NULL) {
		return -ENOMEM;
	}

	(*data)->id = tid;
	(*data)->data_size = size;

	return 0;
}

void dl_free_track_data_buffer(struct log_data *data)
{
	k_heap_free(&mem_pool, data);
}

int dl_add_track_data(struct log_data *data)
{
	struct fifo_item *item;

	item = k_heap_alloc(&mem_pool, FIFO_ITEM_SIZE, K_NO_WAIT);
	if (item == NULL) {
		return -ENOMEM;
	}

	item->cmd = LOG_TRACK_DATA;
	item->data = data;

	k_fifo_put(&fifo, item);
	return 0;
}

int dl_add_track_names_chunk(uint8_t tid, const char *names)
{
	struct fifo_item *item;

	item = k_heap_alloc(&mem_pool, FIFO_ITEM_SIZE, K_NO_WAIT);
	if (item == NULL) {
		return -ENOMEM;
	}

	item->data = k_heap_alloc(&mem_pool, sizeof(struct log_data) +
				  strlen(names) + 1, K_NO_WAIT);

	if (item->data == NULL) {
		return -ENOMEM;
	}

	item->data->data_size = strlen(names) + 1;
	strncpy(item->data->data, names, item->data->data_size);
	item->data->id = tid;

	item->cmd = LOG_TRACK_NAMES;

	k_fifo_put(&fifo, item);
	return 0;
}

int dl_start_track(uint8_t tid, const char *names, const char *fmt)
{
	int rc = dl_add_track_format_chunk(tid, fmt);
	if (rc != 0) {
		return rc;
	}

	return dl_add_track_names_chunk(tid, names);
}

static int open_log(const char *path)
{
	int res;
	res = fs_open(&fd, path, FS_O_WRITE | FS_O_CREATE);
	if (res == 0) {
		log_open = true;
		return res;
	}

	log_open = false;
	return res;
}

static int close_log()
{
	log_open = false;
	fs_sync(&fd);
	return fs_close(&fd);
}

static int add_track_format_chunk(uint8_t tid, const char *format)
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
	
	rc = fs_sync(&fd);
	return rc;
}

static int add_track_names_chunk(uint8_t tid, const char *names)
{
	int rc;
	uint8_t bf = ((2 & 0xF) << 4) | (tid & 0xF);
	rc = fs_write(&fd, &bf, sizeof(uint8_t));
	if (rc != sizeof(uint8_t)) {
		return -EIO;
	}

	size_t dl = strlen(names) + 1;
	rc = fs_write(&fd, names, dl);
	if (rc != dl) {
		return -EIO;
	}

	rc = fs_sync(&fd);
	return rc;
}

static int add_track_data(uint8_t tid, void *data, size_t length)
{
	int rc;
	uint8_t bf = ((3 & 0xF) << 4) | (tid & 0xF);
	rc = fs_write(&fd, &bf, sizeof(uint8_t));
	if (rc != sizeof(uint8_t)) {
		return -EIO;
	}

	rc = fs_write(&fd, data, length);
	if (rc != length) {
		return -EIO;
	}

	//rc = fs_sync(&fd);
	return rc;
}

static int add_file(uint8_t fid, const char *path)
{
	int rc;
	struct fs_dirent entry;
	struct fs_file_t file;
	uint8_t bf = ((4 & 0xF) << 4) | (fid & 0xF);
	uint32_t size;

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

	rc = fs_open(&file, path, FS_O_READ);
	if (rc != 0) {
		LOG_ERR("can not open file");
		return rc;
	}

	while (1) {
		uint8_t buf;
		rc = fs_read(&file, &buf, 1);
		if (rc == 0) {
			break;
		} else if (rc < 0) {
			goto out;
		}
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

void datalogger_consumer(void *arg1, void *arg2, void *arg3)
{
	struct fifo_item *item;
	int res;
	uint32_t max_cylce = 0;
	while (1) {
		item = k_fifo_get(&fifo, K_FOREVER);
		if (item == NULL) {
			continue;
		}
		uint32_t s = k_cycle_get_32();
		LOG_DBG("received new data item");
		LOG_DBG("cmd %d", item->cmd);

		if (item->cmd == LOG_FILE) {
			struct log_data *data;
			data = item->data;

			res = add_file(data->id, data->data);
			k_heap_free(&mem_pool, data);

			if (res != 0) {
				/* TODO: What to do with errors */
			}
		} else if (item->cmd == LOG_TRACK_DATA) {
			struct log_data *data;
			data = item->data;

			res = add_track_data(data->id, data->data,
					     data->data_size);
			k_heap_free(&mem_pool, data);

			if (res != 0) {}
		} else if (item->cmd == LOG_TRACK_FORMAT) {
			struct log_data *data;
			data = item->data;
			LOG_DBG("format: %s", data->data);
			res = add_track_format_chunk(data->id, data->data);
			k_heap_free(&mem_pool, data);

			if (res != 0) {}
		} else if (item->cmd == LOG_TRACK_NAMES) {
			struct log_data *data;
			data = item->data;

			res = add_track_names_chunk(data->id, data->data);
			k_heap_free(&mem_pool, data);

			if (res != 0) {}
		} else if (item->cmd == OPEN_LOG) {
			LOG_DBG("open log file with filename %s",
				current_path);
			res = open_log(current_path);
		} else if (item->cmd == CLOSE_LOG) {
			res = close_log();
		} else {
			LOG_ERR("cmd not known");
		}
		uint32_t c = k_cycle_get_32() - s;
		if(c > max_cylce) {
			max_cylce = c;
			
		}
		
		k_heap_free(&mem_pool, item);
	}
}

K_THREAD_DEFINE(dl_tid, CONFIG_DATALOGGER_STACK_SIZE, datalogger_consumer, NULL,
		NULL, NULL, CONFIG_DATALOGGER_THREAD_PRIORITY, 0, 0);


#endif