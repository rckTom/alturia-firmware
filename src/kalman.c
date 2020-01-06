/*
 * Alturia Firmware - The firmware for the alturia flight computer
 *
 * Copyright (c) Thomas Schmid, 2019
 *
 * Authors:
 *  Thomas Schmid <tom@lfence.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 3.  See
 * the COPYING file in the top-level directory.
 */

#include "kalman.h"

#define DEFINE_MATRIX(NAME, M, N) \
	float32_t NAME_data[M*N]; \
	arm_matrix_instance_f32 NAME = {M, N, &NAME_data};

#define MATRIX_INDEX(i,j,mat) i*mat->numCols+j

/**
 * mat_set() - Set element [i,j] of matrix mat to the specified value
 * @mat: Pointer to a matrix instance
 * @value: Value
 * @i: row index
 * @j: column index
 */
static inline void mat_set(arm_matrix_instance_f32 *mat, float32_t value,
			int i, int j)
{
	mat->pData[MATRIX_INDEX(i,j,mat)] = value;
}

/**
 * mat_mult_ABC() - Computed the matrix multiplication A*B*C
 * @A: Matrix A
 * @B: Matrix B
 * @C: Matrix C
 *
 * Return: return value < 0 indicates errors
 */
static int mat_mult_ABC(const arm_matrix_instance_f32 *A,
		const arm_matrix_instance_f32 *B,
		const arm_matrix_instance_f32 *C,
		arm_matrix_instance_f32 *result)
{
	float32_t inter_data[__KALMAN_MAX_ORDER * __KALMAN_MAX_ORDER];
	arm_matrix_instance_f32 inter;
	arm_status res;

	arm_mat_init_f32(&inter, A->numRows, C->numCols, inter_data);
	res = arm_mat_mult_f32(A, B, &inter);
	if (res != ARM_MATH_SUCCESS) {
		return res;
	}

	res = arm_mat_mult_f32(&inter, C, result);
	return res;
}

/**
 * mat_resize() - Set matrix row and column count
 * @mat: matrix to change
 * @rows: new row count
 * @columns: new column count
 */
static void mat_resize(arm_matrix_instance_f32 *mat, int rows, int cols)
{
	mat->numRows = rows;
	mat->numCols = cols;
}

/**
 * mat_copy_size() - Set size of matrix mat_dest to size of matrix src
 * @mat_dest: matrix to change size of
 * @src: matrix to copy size from
 */
static void mat_copy_size(arm_matrix_instance_f32 *mat_dest,
			const arm_matrix_instance_f32 *src)
{
	mat_dest->numCols = src->numCols;
	mat_dest->numRows = src->numRows;
}

/**
 * mat_eye_sub_A() - Computes the matrix expression I-A, where I is the identity
 * matrix
 */
static void mat_eye_sub_A(const arm_matrix_instance_f32 *A)
{
	for (int i = 0; i < A->numRows; i++) {
		for (int j = 0; j <  A->numCols; j++) {
			if (i == j) {
				A->pData[MATRIX_INDEX(i,j,A)] =
					1 - A->pData[MATRIX_INDEX(i,j,A)];
				continue;
			}
			A->pData[MATRIX_INDEX(i,j,A)] =
					-A->pData[MATRIX_INDEX(i,j,A)];
		}
	}
}

int kalman_predict(struct kalman_sys *sys, const arm_matrix_instance_f32 *u)
{
	arm_status res;
	float32_t xb_data[__KALMAN_MAX_ORDER * __KALMAN_MAX_ORDER];
	float32_t xx_data[__KALMAN_MAX_ORDER * __KALMAN_MAX_ORDER];
	arm_matrix_instance_f32 xb;
	arm_matrix_instance_f32 xx;

	if (u != NULL) {
		arm_mat_init_f32(&xx, sys->A.numRows, 1, xx_data);
		arm_mat_init_f32(&xb, sys->A.numRows, 1, xb_data);

		res = arm_mat_mult_f32(&sys->A, &sys->x_cor, &xx);
		if (res != ARM_MATH_SUCCESS) {
			goto math_error;
		}

		res = arm_mat_mult_f32(&sys->B, u, &xb);
		if (res != ARM_MATH_SUCCESS) {
			goto math_error;
		}

		res = arm_mat_add_f32(&xx, &xb,&sys->x_pre);
		if (res != ARM_MATH_SUCCESS) {
			goto math_error;
		}

	} else {
		res = arm_mat_mult_f32(&sys->A, &sys->x_cor, &sys->x_pre);
		if (res != ARM_MATH_SUCCESS) {
			goto math_error;
		}
	}


	arm_mat_init_f32(&xx, sys->A.numRows, sys->AT.numCols, xx_data);
	arm_mat_init_f32(&xb, sys->G.numRows, sys->GT.numCols, xb_data);

	res = mat_mult_ABC(&sys->A, &sys->P_cor, &sys->AT, &xx);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}

	res = mat_mult_ABC(&sys->G, &sys->Q, &sys->GT, &xb);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}

	res = arm_mat_add_f32(&xx, &xb, &sys->P_pre);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}

	return 0;

math_error:
	LOG_ERR("Math error. Error code %d", res);
	return res;
}

int kalman_correct(struct kalman_sys *sys, const arm_matrix_instance_f32 *u)
{
	arm_status res;

	float32_t temp1_data[__KALMAN_MAX_ORDER * __KALMAN_MAX_ORDER];
	float32_t temp2_data[__KALMAN_MAX_ORDER * __KALMAN_MAX_ORDER];
	float32_t temp3_data[__KALMAN_MAX_ORDER];

	arm_matrix_instance_f32 temp1;
	arm_matrix_instance_f32 temp2;
	arm_matrix_instance_f32 temp3;

	temp1.pData = temp1_data;
	temp2.pData = temp2_data;
	temp3.pData = temp3_data;

	/*
	 * Calculate Kalman gain K
	 * K ( k )= P̂ ( k ) · (C T · C · P̂ ( k ) · C T + R ( k )) ^ − 1
	 */

	mat_resize(&temp1, sys->C.numRows, sys->CT.numCols);
	res = mat_mult_ABC(&sys->C, &sys->P_pre, &sys->CT, &temp1);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}

	mat_copy_size(&temp2, &sys->R);
	res = arm_mat_add_f32(&temp1, &sys->R, &temp2);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}

	mat_copy_size(&temp1, &temp2);
	res = arm_mat_inverse_f32(&temp2, &temp1);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}

	res = mat_mult_ABC(&sys->P_pre, &sys->CT, &temp1, &sys->K);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}

	/*
	 * Calculate corrected state vector x̃
	 * x̃ ( k ) = x̂ ( k ) + K ( k ) · y ( k ) − C · x̂ ( k ) − D · u ( k )
	 */

	if (u != NULL) {
		mat_resize(&temp1, sys->D.numRows, u->numCols);
		res = arm_mat_mult_f32(&sys->D, u, &temp1);
		if (res != ARM_MATH_SUCCESS) {
			goto math_error;
		}

		mat_resize(&temp2, sys->C.numRows, sys->x_pre.numCols);
		res = arm_mat_mult_f32(&sys->C, &sys->x_pre, &temp2);
		if (res != ARM_MATH_SUCCESS) {
			goto math_error;
		}

		mat_copy_size(&temp3, &sys->y);
		res = arm_mat_sub_f32(&sys->y, &temp2, &temp3);
		if (res != ARM_MATH_SUCCESS) {
			goto math_error;
		}

		mat_copy_size(&temp2, &temp3);
		res = arm_mat_sub_f32(&temp3, &temp1, &temp2);
		if (res != ARM_MATH_SUCCESS) {
			goto math_error;
		}
	} else {
		mat_resize(&temp1, sys->C.numRows, sys->x_pre.numCols);
		res = arm_mat_mult_f32(&sys->C, &sys->x_pre, &temp1);
		if (res != ARM_MATH_SUCCESS) {
			goto math_error;
		}

		mat_copy_size(&temp2, &sys->y);
		res = arm_mat_sub_f32(&sys->y, &temp1, &temp2);
		if (res != ARM_MATH_SUCCESS) {
			goto math_error;
		}
	}

	mat_resize(&temp1, sys->K.numRows, temp2.numCols);
	res = arm_mat_mult_f32(&sys->K, &temp2, &temp1);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}

	res = arm_mat_add_f32(&sys->x_pre, &temp1, &sys->x_cor);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}


	/*
	 * Calculate corrected covariance matrix
	 * P̃ ( k ) = I − K ( k ) · C · P̂ ( k )
	 */

	mat_resize(&temp1, sys->K.numRows, sys->C.numCols);
	res = arm_mat_mult_f32(&sys->K, &sys->C, &temp1);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}

	mat_eye_sub_A(&temp1);

	res = arm_mat_mult_f32(&temp1, &sys->P_pre, &sys->P_cor);
	if (res != ARM_MATH_SUCCESS) {
		goto math_error;
	}

	return 0;

math_error:
	LOG_ERR("Math error. Error code %d", res);
	return res;
}

void kalman_sys_init(struct kalman_sys *sys, const arm_matrix_instance_f32 A,
		const arm_matrix_instance_f32 B,
		const arm_matrix_instance_f32 C,
		const arm_matrix_instance_f32 D,
		const arm_matrix_instance_f32 R,
		const arm_matrix_instance_f32 G)
{
	arm_status res;

	mat_resize(&sys->AT, sys->A.numCols, sys->A.numRows);
	mat_resize(&sys->CT, sys->C.numCols, sys->C.numRows);
	mat_resize(&sys->GT, sys->G.numCols, sys->G.numRows);
	res = arm_mat_trans_f32(&sys->A, &sys->AT);
	res = arm_mat_trans_f32(&sys->C, &sys->CT);
	res = arm_mat_trans_f32(&sys->G, &sys->GT);
}
