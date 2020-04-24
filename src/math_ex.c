#include "math_ex.h"
#include <math.h>

float norm2(arm_matrix_instance_f32 *vec)
{
	float32_t sum = 0.f;

	for(int i = 0; i < (vec->numCols * vec->numRows); i++) {
		sum += vec->pData[i] * vec->pData[i];
	}

	return sqrtf(sum);
}
