#ifndef __ALTURIA_KALMAN_H__
#define __ALTURIA_KALMAN_H__

#include "arm_math.h"

#ifndef KALMAN_MAX_ORDER
#define __KALMAN_MAX_ORDER 6
#else
#define __KALMAN_MAX_ORDER KALMAN_MAX_ORDER
#endif

struct kalman_sys{
	arm_matrix_instance_f32 A;
	arm_matrix_instance_f32 B;
	arm_matrix_instance_f32 C;
	arm_matrix_instance_f32 D;
	arm_matrix_instance_f32 x_pre;
	arm_matrix_instance_f32 x_cor;
	arm_matrix_instance_f32 P_pre;
	arm_matrix_instance_f32 P_cor;
};

void kalman_sys_step(struct kalman_sys *sys, arm_matrix_instance_f32 *u);

#endif
