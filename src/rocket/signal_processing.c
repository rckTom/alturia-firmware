#include "daq.h"
#include "kalman_filter.h"
#include "attitude_estimation.h"
#include "transformations_impl.h"
#include "arm_math.h"
#include "is_atmosphere.h"
#include "rocket/signal_processing.h"
#include "rocket/flightstate.h"
#include <zephyr/logging/log.h>
#include "signals.h"
#include "generic.h"
#include "kalman_filter.h"
#include "events2.h"
#include "pt1.h"

LOG_MODULE_DECLARE(alturia, 4);

DEFINE_EVENT(event_processing_ready);

static struct daq_sample sample;

static struct pt1_state altitude_offset_filter;
static struct pt1_state acc_x_offset_filter;
static struct pt1_state acc_y_offset_filter;
static struct pt1_state acc_z_offset_filter;

static const uint32_t offset_samples = 100;
static uint32_t offset_sample_counter = 0;

static float32_t altitude_raw, altitude_filterd, offset_altitude, altitude_corrected, offset_acc_x, offset_acc_y, offset_acc_z;

STATIC_MATRIX(a_raw, 3, 1);
STATIC_MATRIX(a_w, 3, 1);
STATIC_MATRIX(omega_raw, 3, 1);
STATIC_MATRIX(omega_w, 3, 1);
STATIC_MATRIX(v_w, 3, 1);

/* Parameters for basic kalman filter */
STATIC_MATRIX(xpre_kalman_basic, 2, 1);
STATIC_MATRIX(Ppre_kalman_basic, 2, 2);
STATIC_MATRIX(xcor_kalman_basic, 2, 1);
STATIC_MATRIX(Pcor_kalman_basic, 2, 2);

/* Parameters for vertical dynamics kalman filter */
STATIC_MATRIX(xpre_kalman_vertical, 3, 1);
STATIC_MATRIX(Ppre_kalman_vertical, 3, 3);
STATIC_MATRIX(xcor_kalman_vertical, 3, 1);
STATIC_MATRIX(Pcor_kalman_vertical, 3, 3);

/* Parameters for attitude estimation */
STATIC_MATRIX(q, 4, 1);
STATIC_MATRIX(gyro_bias, 3, 1);

DEFINE_SIGNAL(signal_pressure, INIT_GENERIC(type_float32, &sample.pressure));
DEFINE_SIGNAL(signal_h_raw, INIT_GENERIC(type_float32, &altitude_raw));
DEFINE_SIGNAL(signal_h, INIT_GENERIC(type_float32, &altitude_filterd));
DEFINE_SIGNAL(signal_v_w, INIT_GENERIC(type_matrix, &v_w));
DEFINE_SIGNAL(signal_a_raw, INIT_GENERIC(type_matrix, &a_raw));
DEFINE_SIGNAL(signal_a_w, INIT_GENERIC(type_matrix, &a_w));
DEFINE_SIGNAL(signal_kalman_basic, INIT_GENERIC(type_matrix, &xcor_kalman_basic));
DEFINE_SIGNAL(signal_omega_raw, INIT_GENERIC(type_matrix, &omega_raw));
DEFINE_SIGNAL(signal_omega_w, INIT_GENERIC(type_matrix, &omega_w));
DEFINE_SIGNAL(signal_h_offset, INIT_GENERIC(type_float32, &offset_altitude));
DEFINE_SIGNAL(signal_q, INIT_GENERIC(type_matrix, &q));

DECLARE_EVENT(event_liftoff);
DECLARE_EVENT(event_ignition);

static enum signal_processing_state state = SIGNAL_PROCESSING_INACTIVE;
static enum signal_processing_attitude_estimator attitude_estimator = ATTITUDE_ESTIMATION_MAHOHNY;
static enum signal_processing_altitude_kalman_filter kalman_filter = KALMAN_CONST_ACC;

float32_t incremental_average(float32_t average, float32_t count, float32_t value)
{
    return (average + (value-average)/count);
}

static bool sample_offsets()
{
    offset_sample_counter++;
    offset_altitude = incremental_average(offset_altitude, offset_sample_counter, altitude_raw);

    if (offset_sample_counter >= offset_samples) {
        return true;
    }

    return false;
}

enum signal_processing_state signal_processing_get_state()
{
    return state;
}

void signal_processing_set_state(enum signal_processing_state s) {
    state = s;
}

void signal_processing_init()
{
    mat_identity(&Pcor_kalman_basic);
    mat_identity(&Ppre_kalman_basic);
    mat_identity(&Pcor_kalman_vertical);
    mat_identity(&Ppre_kalman_vertical);
    mat_zero(&q);
    mat_zero(&gyro_bias);
    mat_zero(&v_w);

    mat_set(q, 0, 0, 1.0f);
    mat_set(xcor_kalman_basic, 0, 0, 0.0f);
    mat_set(xcor_kalman_basic, 1, 0, 0.0f);

    offset_sample_counter = 0;
    pt1_init(&altitude_offset_filter, 0, 1.0f, 10.0f);
    pt1_init(&acc_x_offset_filter, 0, 1.0F, 5.0f);
    pt1_init(&acc_y_offset_filter, 0, 1.0F, 5.0f);
    pt1_init(&acc_z_offset_filter, 0, 1.0F, 5.0f);
    state = SIGNAL_PROCESSING_STARTING;
}

static void run_constant_velocity_kalman_filter(float dt)
{
    altitude_kal_pre(&xcor_kalman_basic,
                        &Pcor_kalman_basic,
                        dt, 0.02f,
                        &xpre_kalman_basic,
                        &Ppre_kalman_basic);
    altitude_kal_cor(&xpre_kalman_basic,
                        &Ppre_kalman_basic,
                        0.3f, altitude_corrected,
                        &xcor_kalman_basic,
                        &Pcor_kalman_basic);
    altitude_filterd = mat_get(xcor_kalman_basic, 0, 0);
    mat_set(v_w, 2, 0, mat_get(xcor_kalman_basic, 1, 0));
}

static void run_constant_acceleration_kalman_filter(float dt)
{
    //vertical_dyn_kal_pre();
    //vertical_dyn_kal_cor();
}

static void run_position_integration(float dt)
{

}

static void run_attitude_integration(float dt)
{
    attitude_simple_q_integration(&q, &omega_raw, &gyro_bias, dt);
}

static void run_mahony_filter(float dt)
{
    mahony_ahrs(&a_raw, &omega_raw, &gyro_bias, &q, dt, 0.3f, 1.0f);
}

int signal_processing_main() {
    int rc = 0;
    float32_t dt = 0.01f;
    static float v_w_last = 0;

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
    
    trans_ACC_to_W(a_raw.pData, a_w.pData);
    trans_IMU_to_W(omega_raw.pData, omega_w.pData);

    if (state == SIGNAL_PROCESSING_STARTING) {

        if(sample_offsets()) {
            state = SIGNAL_PROCESSING_ON_GROUND;
            event2_fire(&event_processing_ready);

            altitude_offset_filter.y_n1 = offset_altitude;
            altitude_corrected = altitude_raw - offset_altitude;
            mat_set(xcor_kalman_basic, 0, 0, altitude_corrected);
            mat_set(xcor_kalman_basic, 1, 0, 0.0f);
            mat_set(q, 0, 0, 1.0f);   
            
        }
        goto out_release_sched_lock;
    } 
    if (state == SIGNAL_PROCESSING_ON_GROUND || state == SIGNAL_PROCESSING_IN_FLIGHT) {
        //update altitude offset with very slow pt1 filter
        if (!flightstate_inflight()) {
            offset_altitude = pt1_update(&altitude_offset_filter, altitude_raw, dt);
            offset_acc_x = pt1_update(&acc_x_offset_filter, mat_get(a_w, 0, 0), dt);
            offset_acc_y = pt1_update(&acc_x_offset_filter, mat_get(a_w, 1, 0), dt);
            offset_acc_z = pt1_update(&acc_x_offset_filter, mat_get(a_w, 2, 0), dt);
        }

        //attitude estimation
        mahony_ahrs(&a_raw, &omega_raw, &gyro_bias, &q, dt, 0.3f, 1.0f);
        altitude_corrected = altitude_raw - offset_altitude;
        run_constant_velocity_kalman_filter(dt);

        //euler derivative of velocity to get acceleration
        float a = (mat_get(v_w, 2, 0)-v_w_last)/dt;
    }
out_release_sched_lock:
    k_sched_unlock();
out_final:
    return rc;
}

