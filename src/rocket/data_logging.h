#include <zephyr/zephyr.h>
#include "stdint.h"
#include "stdlib.h"
#include "arm_math.h"

struct log_entry_sensor_track {
	int64_t timestamp;
	float32_t pressure;
	float32_t h_raw;
	float32_t h_f;
	float32_t v_w;
	float32_t ax;
	float32_t ay;
	float32_t az;
	float32_t gx;
	float32_t gy;
	float32_t gz;
	float32_t qw;
	float32_t qx;
	float32_t qy;
	float32_t qz;
} __attribute__((packed));

struct log_entry_event_track {
	int64_t timestamp;
	uint32_t event_id;
} __attribute__((packed));

void rocket_data_logging();
void rocket_data_logging_set_name(char *name);
void rocket_data_logging_stop();