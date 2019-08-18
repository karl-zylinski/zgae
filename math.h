#pragma once
#include "math_types.h"

#define PI 3.14159265358979323846
mat4_t mat4_create_projection_matrix(float bb_width, float bb_height);
mat4_t mat4_identity();
mat4_t mat4_from_rotation_and_translation(const quat_t* q, const vec3_t* t);
mat4_t mat4_mul(const mat4_t* m1, const mat4_t* m2);
mat4_t mat4_inverse(const mat4_t* m);
quat_t quat_identity();