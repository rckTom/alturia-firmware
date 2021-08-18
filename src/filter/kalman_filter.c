#include "kalman_filter.h"
#include "math_ex.h"
#include <arm_math.h>

#include "altitude_kalman_impl.h"
#include "constant_altitude_kalman_impl.h"
#include "vertical_dynamics_kalman_impl.h"

void constant_altitude_kal_cor(float xpre, float Ppre, float v_meas, float y,
			       float *x_cor, float *Pcor)
{
	constant_altitude_kalman_impl_correct(&Ppre, v_meas, &xpre, &y, Pcor,
					      x_cor);
}

void constant_altitude_kal_pre(float xcor, float Pcor, float v_sys, float *xpre,
			       float *Ppre)
{
	constant_altitude_kalman_impl_predict(&Pcor, v_sys, &xcor, Ppre, xpre);
}

void altitude_kal_pre(mat *xcor,
		      mat *Pcor, float dt,
		      float v_sys_pressure, mat *xpre,
		      mat *Ppre)
{
	altitude_kalman_impl_predict(Pcor->pData, dt, v_sys_pressure,
				     xcor->pData, Ppre->pData, xpre->pData);
}

void altitude_kal_cor(mat *xpre,
		      mat *Ppre, float v_meas_altitude,
		      float altitude, mat *xcor,
		      mat *Pcor)
{
	altitude_kalman_impl_correct(Ppre->pData, v_meas_altitude, xpre->pData,
				     &altitude, Pcor->pData, xcor->pData);
}

void vertical_dyn_kal_cor(mat *P_pre, float variance_meas_a,
			  float variance_meas_h, mat *x_pre, mat *y, mat *P_cor,
			  mat *x_cor)
{
	vertical_dynamics_kalman_impl_correct(
	    P_pre->pData, variance_meas_a, variance_meas_h, x_pre->pData,
	    y->pData, P_cor->pData, x_cor->pData);
}

void vertical_dyn_kal_pre(mat *P_cor, float dt, float variance_acc,
			  float variance_h, mat *x_cor, mat *P_pre, mat *x_pre)
{
	vertical_dynamics_kalman_impl_predict(P_cor->pData, dt, variance_acc,
					      variance_h, x_cor->pData,
					      P_pre->pData, x_pre->pData);
}
