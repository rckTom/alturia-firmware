#ifndef ALTURIA__KALMAN_FILTER__H
#define ALTURIA__KALMAN_FILTER__H

void constant_altitude_kal_cor(float xpre, float Ppre,
			       float v_meas, float y, float *x_cor,
			       float *Pcor);

void constant_altitude_kal_pre(float xcor, float Pcor,
			       float v_sys, float *xpre, float *Ppre);

#endif /* ALTURIA__KALMAN_FILTER__H */
