#ifndef ALTURIA__EDGE_DETECTOR__H
#define ALTURIA__EDGE_DETECTOR__H

#include <stdbool.h>
#include <stdint.h>

typedef bool (*condition_fcn) (float signal_value, struct edge_detector *trig, void *user_data);

struct edge_detector {
    int64_t last_edge_time;
    int64_t min_hold_time;
    void *user_data;
    condition_fcn cond;
    bool last_state;
};

void edge_detector_init(struct edge_trigger *dat, condition_fcn cond, void *user_data, int64_t min_hold_time);
bool edge_detector_update(struct edge_detector *dat, int64_t time);

bool edge_detector_cond_gt(float signal_value, void *user_data);
bool edge_detector_cond_st(float signal_value, void *user_data);

#endif