#include "kalman_filter.h"
#include "arm_math.h"

#include "constant_altitude_kalman_impl.h"

void constant_altitude_kal_cor(float xpre, float Ppre,
			       float v_meas, float y, float *x_cor,
			       float *Pcor)
{
	constant_altitude_kalman_impl_correct(&Ppre, v_meas, &xpre, &y,
					      Pcor, x_cor);
}

void constant_altitude_kal_pre(float xcor, float Pcor,
			       float v_sys, float *xpre, float *Ppre)
{
	constant_altitude_kalman_impl_predict(&Pcor, v_sys, &xcor, Ppre, xpre);
}
