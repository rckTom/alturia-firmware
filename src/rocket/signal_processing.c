#include "daq.h"
#include "kalman_filter.h"
#include "arm_math.h"
#include "is_atmosphere.h"
#include "rocket/signal_processing.h"
#include "logging/log.h"
#include "signals.h"
#include "generic.h"
#include "kalman_filter.h"

LOG_MODULE_DECLARE(alturia, 4);

static struct daq_sample sample;
static enum signal_processing_state state = SIGNAL_PROCESSING_INACTIVE;

static const uint32_t offset_samples = 100;
static uint32_t offset_sample_counter = 0;

static float32_t altitude_raw, altitude_filterd, vertical_speed, offset_altitude;
static float32_t offsets_gyro[3];
static float32_t offsets_acceleration[3];

STATIC_MATRIX(a_raw, 3, 1);
STATIC_MATRIX(omega_raw, 3, 1);
STATIC_MATRIX(v_w, 3, 1);


/* Parameters for basic kalman filter */
STATIC_MATRIX(xpre_kalman_basic, 2, 1);
STATIC_MATRIX(Ppre_kalman_basic, 2, 2);
STATIC_MATRIX(xcor_kalman_basic, 2, 1);
STATIC_MATRIX(Pcor_kalman_basic, 2, 2);


float32_t incremental_average(float32_t average, float32_t count, float32_t value)
{
    return (average + (value-average)/count);
}

static bool sample_offsets()
{
    offset_sample_counter++;
    offset_altitude = incremental_average(offset_altitude, offset_sample_counter, altitude_raw);

    if (offset_sample_counter == offset_samples) {
        return true;
    }

    return false;
}

enum signal_processing_state signal_processing_get_state()
{
    return state;
}

void signal_processing_init()
{
    signal_insert("pressure", generic_ptr_from_float32(&sample.pressure));
    signal_insert("h_raw", generic_ptr_from_float32(&altitude_raw));
    signal_insert("h", generic_ptr_from_float32(&altitude_filterd));
    signal_insert("v_w", generic_ptr_from_matrix(&v_w));
    signal_insert("a_raw", generic_ptr_from_matrix(&a_raw));
    signal_insert("x_kalman_basic", generic_ptr_from_matrix(&xcor_kalman_basic));
    signal_insert("omega_raw", generic_ptr_from_matrix(&omega_raw));

    mat_identity(&Pcor_kalman_basic);
    mat_zero(&v_w);
    offset_sample_counter = 0;
    state = SIGNAL_PROCESSING_STARTING;
}

int signal_processing_main() {
    int rc = 0;
    float32_t dt = 0.01f;

    if (state == SIGNAL_PROCESSING_INACTIVE) {
        goto out_final;
    }

    rc = daq_sync();
    if (rc != 0) {
        LOG_ERR("unable to sync to daq");
        goto out_final;
    }

    daq_get_sample(&sample);

    k_sched_lock(); 

    altitude_raw = isa_calc_height(sample.pressure * 1000);
    mat_set(a_raw, 0, 0, sample.acc_x);
    mat_set(a_raw, 1, 0, sample.acc_y);
    mat_set(a_raw, 2, 0, sample.acc_z);
    mat_set(omega_raw, 0, 0, sample.gyro_x);
    mat_set(omega_raw, 1, 0, sample.gyro_y);
    mat_set(omega_raw, 2, 0, sample.gyro_z);

    if (state == SIGNAL_PROCESSING_STARTING) {

        if(sample_offsets()) {
            state == SIGNAL_PROCESSING_ACTIVE;
            altitude_raw -= offset_altitude;
            mat_set(xcor_kalman_basic, 0, 0, altitude_raw);
            mat_set(xcor_kalman_basic, 1, 0, 0.0f);
        }

        goto out_release_sched_lock;
    } else if (state == SIGNAL_PROCESSING_ACTIVE) {
        //build state vector for kalman filter

        altitude_kal_pre(&xcor_kalman_basic,
                         &Pcor_kalman_basic,
                         dt, 0,
                         &xpre_kalman_basic,
                         &Ppre_kalman_basic);
        altitude_kal_cor(&xpre_kalman_basic,
                         &Ppre_kalman_basic,
                         0, altitude_raw,
                         &xcor_kalman_basic,
                         &Pcor_kalman_basic);
        altitude_filterd = mat_get(xcor_kalman_basic, 0, 0);
        mat_set(v_w, 2, 1, mat_get(xcor_kalman_basic, 1, 0));
    }

out_release_sched_lock:
    k_sched_unlock();
out_final:
    return rc;
}

