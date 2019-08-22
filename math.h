#pragma once
#include "math_types.h"

#define PI 3.14159265358979323846
Mat4 mat4_create_projection_matrix(f32 bb_width, f32 bb_height);
Mat4 mat4_identity();
Mat4 mat4_from_rotation_and_translation(const Quat* q, const Vec3* t);
Mat4 mat4_mul(const Mat4* m1, const Mat4* m2);
Mat4 mat4_inverse(const Mat4* m);
Quat quat_identity();

bool f32_almost_eql(f32 f1, f32 f2);
bool vec2_almost_eql(const Vec2* v1, const Vec2* v2);
bool vec3_almost_eql(const Vec3* v1, const Vec3* v2);
bool vec4_almost_eql(const Vec4* v1, const Vec4* v2);