#pragma once
#include "math_types.h"

#define PI 3.14159265358979323846
#define SMALL_NUMBER 0.00001f

Mat4 mat4_create_projection_matrix(f32 bb_width, f32 bb_height);
Mat4 mat4_identity();
Mat4 mat4_from_rotation_and_translation(const Quat& q, const Vec3& t);
Mat4 mat4_inverse(const Mat4& m);
f32 dot(const Vec3& v1, const Vec3& v2);
Vec3 normalize(const Vec3& v);
f32 len(const Vec3& v);

Mat4 operator*(const Mat4& m1, const Mat4& m2);
Vec3 operator-(const Vec3& v);
Vec3 operator-(const Vec3& v1, const Vec3& v2);
Vec3 operator*(const Vec3& v, f32 s);

Vec3 operator+(const Vec3& v1, const Vec3& v2);
void operator+=(Vec3& v1, const Vec3& v2);

Vec3 cross(const Vec3& v1, const Vec3& v2);
bool operator==(const Vec3& v1, const Vec3& v2);

bool f32_almost_eql(f32 f1, f32 f2);
bool vec2_almost_eql(const Vec2& v1, const Vec2& v2);
bool vec3_almost_eql(const Vec3& v1, const Vec3& v2);
bool vec4_almost_eql(const Vec4& v1, const Vec4& v2);

Quat quat_identity();
Quat operator*(const Quat& a, const Quat& b);
Quat quat_rotate_x(const Quat& q, float rads);
Quat quat_rotate_y(const Quat& q, float rads);
Quat quat_rotate_z(const Quat& q, float rads);
Quat quat_from_axis_angle(const Vec3& axis, float angle);
Quat quat_normalize(const Quat& q);
Vec3 quat_transform_vec3(const Quat&q, const Vec3& v);