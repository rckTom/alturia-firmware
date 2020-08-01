#ifndef ALTURIA_MATH_EX_H
#define ALTURIA_MATH_EX_H

#include "arm_math.h"

typedef arm_matrix_instance_f32 mat;

#define mat_get(mat, row, col) (mat.pData[mat.numCols * row + col])
#define mat_set(mat, row, col, value) (mat.pData[mat.numCols * row + col] = value)

float norm2(mat *vec);


#endif
