#pragma once
#include "math_types.h"

#define PI 3.14159265358979323846
Mat4 mat4_create_projection_matrix(f32 bb_width, f32 bb_height);
Mat4 mat4_identity();
Mat4 mat4_from_rotation_and_translation(const Quat* q, const Vec3* t);
Mat4 mat4_mul(const Mat4* m1, const Mat4* m2);
Mat4 mat4_inverse(const Mat4* m);
Quat quat_identity();