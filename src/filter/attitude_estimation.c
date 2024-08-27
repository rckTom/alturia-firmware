#include "attitude_estimation.h"
#include "arm_math.h"
#include "generated/mahohny_attitude_estimation_impl.h"
#include "generated/attitude_estimation_impl.h"

void attitude_simple_q_integration(mat *q, mat *g_m, mat *gyro_bias, float dt) {
	mat q_next;
	float32_t q_next_data[4] = {0.0f};

	q_next.numCols = 1;
	q_next.numRows = 4;
	q_next.pData = q_next_data;

	attitude_q_next(dt,	gyro_bias->pData, g_m->pData, q->pData, q_next.pData);
	memcpy(q->pData, q_next.pData, (q_next.numCols * q_next.numRows) * sizeof(float32_t));
}

void mahony_ahrs(mat *a_m, mat *g_m, mat *gyro_bias, mat *q, float dt, float ki,
		 float kp)
{
	mat q_next;
	mat bias_next;

	float32_t q_next_data[4] = {0.0f};
	float32_t bias_next_data[3] = {0.0f};

	q_next.numCols = 1;
	q_next.numRows = 4;
	q_next.pData = q_next_data;
	bias_next.numCols = 1;
	bias_next.numRows = 3;
	bias_next.pData = bias_next_data;

	mahony_q_next(a_m->pData, dt, g_m->pData, gyro_bias->pData, ki, kp,
		      q->pData, bias_next.pData, q_next.pData);

	memcpy(q->pData, q_next.pData,
	       q->numCols * q->numRows * sizeof(float32_t));
	memcpy(gyro_bias->pData, bias_next.pData,
	       gyro_bias->numCols * gyro_bias->numRows * sizeof(float32_t));
}
