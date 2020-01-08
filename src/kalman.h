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

#define INIT_MATRIX(sys_name, member_name, rows, cols) \
	float32_t sys_name_member_name_data[rows*cols]; \
	sys_name.member_name.pData = sys_name_member_name_data; \
	sys_name.numRows = rows; \
	sys_name.numCols = cols;

#define DEFINE_KALMAN_FILTER(name, m, n, p) \
	struct kalman_sys name; \
	INIT_MATRIX(name, A, n, n) \
	INIT_MATRIX(name, B, n, m) \
	INIT_MATRIX(name, C, p, n) \
	INIT_MATRIX(name, D, p, m) \
	INIT_MATRIX(name, G, n, m) \
	INIT_MATRIX(name, GT, m, n) \
	INIT_MATRIX(name, AT, n, n) \
	INIT_MATRIX(name, CT, n, p) \

void kalman_sys_step(struct kalman_sys *sys, arm_matrix_instance_f32 *u);

#endif
