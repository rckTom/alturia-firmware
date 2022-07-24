#ifndef ALTURIA__ATTITUDE_ESTIMATION__H
#define ALTURIA__ATTITUDE_ESTIMATION__H

#include "math_ex.h"

void mahony_ahrs(mat *a_m, mat *g_m, mat *gyro_bias, mat *q, float dt, float ki,
		 float kp);
void simple_q_integration();

#endif