#include "datalogger.h"
#include <fs/fs.h>
#include <string.h>
#include <zephyr.h>

static struct fs_file_t fd;

int open_log(const char *path)
{
	return fs_open(&fd, path);
}

int close_log()
{
	return fs_close(&fd);
}

int add_track_format_chunk(int tid, const char* format)
{
	int rc;
	u8_t bf = ((1 & 0xF) << 4) | (tid & 0xF);
	rc = fs_write(&fd, &bf, sizeof(u8_t));
	rc = fs_write(&fd, format, strlen(format) + 1);
	return rc;
}

int add_track_names_chunk(int tid, const char* names)
{
	int rc;
	u8_t bf = ((2 & 0xF) << 4) | (tid & 0xF);
	rc = fs_write(&fd, &bf, sizeof(u8_t));
	rc = fs_write(&fd, names, strlen(names) + 1);
	return rc;
}

int add_track_data(int tid, void *data, size_t length)
{
	int rc;
	u8_t bf = ((3 & 0xF) << 4) | (tid & 0xF);
	rc = fs_write(&fd, &bf, sizeof(u8_t));
	rc = fs_write(&fd, data, length);
	return rc;
}