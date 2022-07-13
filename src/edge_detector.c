#include "edge_detector.h"

void edge_detector_init(struct edge_detector *dat, condition_fcn cond, int64_t min_hold_time)
{
    dat->min_hold_time = min_hold_time;
    dat->cond = cond;
    dat->last_edge_time = -1;
    dat->last_state = false;
}

bool edge_detector_update(struct edge_detector *dat,float value, void * user_data, int64_t time)
{
    bool state = dat->cond(value, dat, user_data);

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

void edge_detector_reset(struct edge_detector *dat)
{
    dat->last_state = false;
    dat->last_edge_time = -1;
}

bool edge_detector_cond_gt(float signal_value, struct edge_detector *dat, void *threshold)
{
    return signal_value >= *(float *)threshold;
}

bool edge_detector_cond_st(float signal_value, struct edge_detector *dat, void *threshold)
{
    return signal_value <= *(float *)threshold;
}

bool edge_detector_cond_window(float value, struct edge_detector *dat, void *vdata)
{
    struct cond_window_data *data = (struct cond_window_data *) vdata;
    return (value <= data->target + data->window_width && value >= data->target - data->window_width);
}