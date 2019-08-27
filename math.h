#pragma once
#include "math_types.h"

#define PI 3.14159265358979323846
Mat4 mat4_create_projection_matrix(f32 bb_width, f32 bb_height);
Mat4 mat4_identity();
Mat4 mat4_from_rotation_and_translation(Quat* q, Vec3* t);
Mat4 mat4_inverse(Mat4* m);
Quat quat_identity();

Mat4 operator*(const Mat4& m1, const Mat4& m2);

bool f32_almost_eql(f32 f1, f32 f2);
bool vec2_almost_eql(Vec2* v1, Vec2* v2);
bool vec3_almost_eql(Vec3* v1, Vec3* v2);
bool vec4_almost_eql(Vec4* v1, Vec4* v2);