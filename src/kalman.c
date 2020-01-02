#include "kalman.h"

#define DEFINE_MATRIX(NAME, M, N) \
	float32_t NAME_data[M*N]; \
	arm_matrix_instance_f32 NAME = {M, N, &NAME_data};

void kalman_sys_step(struct kalman_sys *sys, arm_matrix_instance_f32 *u)
{
	arm_status res;

	if (u != NULL) {
		float32_t xb_data[__KALMAN_MAX_ORDER];
		float32_t xx_data[__KALMAN_MAX_ORDER];
		arm_matrix_instance_f32 xb;
		arm_matrix_instance_f32 xx;

		arm_mat_init_f32(&xx, sys->A.numRows, 1, xx_data);
		arm_mat_init_f32(&xb, sys->A.numRows, 1, xb_data);

		res = arm_mat_mult_f32(&sys->A, &sys->x_cor, &xx);
		res = arm_mat_mult_f32(&sys->B, u, &xb);

		res = arm_mat_add_f32(&xx, &xb,&sys->x_pre);
	} else {
		res = arm_mat_mult_f32(&sys->A, &sys->x_cor, &sys->x_pre);
	}


}

void kalman_sys_init(struct kalman_sys *sys)
{

}
