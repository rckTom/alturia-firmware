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
	arm_matrix_instance_f32 G;
	arm_matrix_instance_f32 GT;
	arm_matrix_instance_f32 AT;
	arm_matrix_instance_f32 CT;
	arm_matrix_instance_f32 Q;
	arm_matrix_instance_f32 R;
	arm_matrix_instance_f32 K;
	arm_matrix_instance_f32 x_pre;
	arm_matrix_instance_f32 x_cor;
	arm_matrix_instance_f32 P_pre;
	arm_matrix_instance_f32 P_cor;
	arm_matrix_instance_f32 y;
};

void kalman_sys_step(struct kalman_sys *sys, arm_matrix_instance_f32 *u);

#endif
