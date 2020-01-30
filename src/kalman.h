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

#ifndef ALTURIA__KALMAN__H
#define ALTURIA__KALMAN__H

#include "arm_math.h"
#include <assert.h>

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

#define KALMAN_SYS_INIT_EMPTY_MATRIX(member_name, rows, cols) \
	.member_name = { \
		.numRows = rows, \
		.numCols = cols, \
		.pData = (float32_t[rows * cols]){}, \
	}

#define KALMAN_SYS_INIT_MATRIX(member_name, rows, cols, content) \
	.member_name = { \
		.numRows = rows, \
		.numCols = cols, \
		.pData = (float32_t[rows * cols])content, \
	}


#define DEFINE_KALMAN_FILTER(name, m, n, p) \
	static_assert(n <= __KALMAN_MAX_ORDER, "Kalman filter order too large");\
	static struct kalman_sys name = { \
		KALMAN_SYS_INIT_EMPTY_MATRIX(A, n, n), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(B, n, m), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(C, p, n), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(D, p, m), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(G, n, m), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(GT, m, n), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(AT, n, n), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(CT, n, p), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(R, m, m), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(Q, m, m), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(K, n, m), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(x_pre, n, 1), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(x_cor, n, 1), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(P_pre, n, n), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(P_cor, n, n), \
		KALMAN_SYS_INIT_EMPTY_MATRIX(y, p, 1) \
	};

int kalman_predict(struct kalman_sys *sys, const arm_matrix_instance_f32 *u);
int kalman_correct(struct kalman_sys *sys, const arm_matrix_instance_f32 *u);

void kalman_sys_step(struct kalman_sys *sys, arm_matrix_instance_f32 *u);

#endif
