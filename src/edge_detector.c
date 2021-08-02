#include "edge_detector.h"

void edge_detector_init(struct edge_detector *dat, condition_fcn cond,
                        void *user_data, int64_t min_hold_time)
{
    dat->min_hold_time = min_hold_time;
    dat->cond = cond;
    dat->last_edge_time = -1;
    dat->user_data = user_data;
    dat->last_state = false;
}

bool edge_detector_update(struct edge_detector *dat, float value, int64_t time)
{
    bool state = dat->cond(value, dat, dat->user_data);

    if (!state) {
        goto out;
    }

    if (!dat->last_state) {
        dat->last_edge_time = time;
    }

    if (time - dat->last_edge_time >= dat->min_hold_time) {
        return true;
    }

out:
    dat->last_state = state;
    return false;
}

bool edge_detector_cond_gt(float signal_value, void *user_data)
{
    float *threshold = user_data;
    return signal_value >= *threshold;
}

bool edge_detector_cond_st(float signal_value, void *user_data)
{
    float *threshold = user_data;
    return signal_value <= *threshold;
}