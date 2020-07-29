#ifndef ALTURIA__KALMAN_FILTER__H
#define ALTURIA__KALMAN_FILTER__H

#include "math_ex.h"
#include <arm_math.h>

void constant_altitude_kal_cor(float xpre, float Ppre, float v_meas, float y,
			       float *x_cor, float *Pcor);

void constant_altitude_kal_pre(float xcor, float Pcor, float v_sys, float *xpre,
			       float *Ppre);

void altitude_kal_pre(arm_matrix_instance_f32 *xcor,
		      arm_matrix_instance_f32 *Pcor, float dt,
		      float v_sys_pressure, arm_matrix_instance_f32 *xpre,
		      arm_matrix_instance_f32 *Ppre);

void altitude_kal_cor(arm_matrix_instance_f32 *xpre,
		      arm_matrix_instance_f32 *Ppre, float v_meas_altitude,
		      float altitude, arm_matrix_instance_f32 *xcor,
		      arm_matrix_instance_f32 *Pcor);

void vertical_dyn_kal_cor(mat *P_pre, float variance_meas_a,
			  float variance_meas_h, mat *x_pre, mat *y, mat *P_cor,
			  mat *x_cor);

void vertical_dyn_kal_pre(mat *P_cor, float dt, float variance_acc,
			  float variance_h, mat *x_cor, mat *P_pre, mat *x_pre);

#endif /* ALTURIA__KALMAN_FILTER__H */
