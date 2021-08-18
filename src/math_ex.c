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

void mat_zero(mat *matrix)
{
	memset(matrix->pData, 0, (matrix->numCols * matrix->numRows) * sizeof(matrix->pData[0]));
}

int mat_identity(mat *matrix)
{
	if (matrix->numCols != matrix->numRows) {
		return -1;
	}

	mat_zero(matrix);
	for (int i = 0; i < matrix->numCols; i++) {
		mat_set((*matrix), i, i, 1.0f);
	}

	return 0;
}
