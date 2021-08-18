#ifndef ALTURIA_MATH_EX_H
#define ALTURIA_MATH_EX_H

#include "arm_math.h"

typedef arm_matrix_instance_f32 mat;


#define STATIC_MATRIX(name, row, col) \
    static float32_t mat_##name##_data[row * col] = {0.0f}; \
    static mat name =  { \
        .pData = mat_##name##_data, \
        .numRows = row, \
        .numCols = col \
    };

#define mat_get(mat, row, col) (mat.pData[mat.numCols * row + col])
#define mat_set(mat, row, col, value) (mat.pData[mat.numCols * row + col] = value)

void mat_zero(mat *matrix);
int mat_identity(mat *matrix);
float norm2(mat *vec);


#endif
