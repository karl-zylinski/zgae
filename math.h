#pragma once
#include "math_types.h"

#define PI 3.14159265358979323846
#define SMALL_NUMBER 0.00001f

Mat4 mat4_create_projection_matrix(f32 bb_width, f32 bb_height);
Mat4 mat4_identity();
Mat4 mat4_from_rotation_and_translation(C(Quat) q, C(Vec3) t);
Mat4 mat4_inverse(C(Mat4) m);
Quat quat_identity();
f32 dot(C(Vec3) v1, C(Vec3) v2);
Vec3 normalize(C(Vec3) v);
f32 len(C(Vec3) v);

Mat4 operator*(C(Mat4) m1, C(Mat4) m2);
Vec3 operator-(C(Vec3) v);
Vec3 operator-(C(Vec3) v1, C(Vec3) v2);
Vec3 operator*(C(Vec3) v, f32 s);

Vec3 cross(C(Vec3) v1, C(Vec3) v2);
bool operator==(C(Vec3) v1, C(Vec3) v2);

bool f32_almost_eql(f32 f1, f32 f2);
bool vec2_almost_eql(C(Vec2) v1, C(Vec2) v2);
bool vec3_almost_eql(C(Vec3) v1, C(Vec3) v2);
bool vec4_almost_eql(C(Vec4) v1, C(Vec4) v2);