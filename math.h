#pragma once

struct Vec2
{
    float x, y;
};

struct Vec2i
{
    int x, y;
};

struct Vec3
{
    float x, y, z;
};

struct Vec4
{
    float x, y, z, w;
};

struct Mat4
{
    Vec4 x, y, z, w;
};

struct Quat
{
    float x, y, z, w;
};

static const float PI = 3.1415926535897932f;

Mat4 operator*(const Mat4& m1, const Mat4& m2);
void operator+=(Vec2& v1, const Vec2& v2);
Vec2 operator+(const Vec2& v1, const Vec2& v2);
void operator+=(Vec2i& v1, const Vec2i& v2);
Vec2i operator+(const Vec2i& v1, const Vec2i& v2);
void operator+=(Vec3& v1, const Vec3& v2);
Vec3 operator-(const Vec3& v);
Vec3 operator+(const Vec3& v1, const Vec3& v2);
Vec3 operator-(const Vec3& v1, const Vec3& v2);
Vec3 operator*(const Vec3& v, float s);
Vec3 operator*(const Vec3& v1, const Vec3& v2);
bool operator==(const Vec3& v1, const Vec3& v2);
bool operator!=(const Vec3& v1, const Vec3& v2);
void operator+=(Vec4& v1, const Vec4& v2);
Vec4 operator+(const Vec4& v1, const Vec4& v2);
Vec4 operator*(const Vec4& v, float s);
Vec4 operator*(const Vec4& v1, const Vec4& v2);
Vec4 operator*(const Vec4& v, const Mat4& m);
Mat4 operator*(const Mat4& m, float s);
Vec4 operator-(const Vec4& v1, const Vec4& v2);
Quat operator*(const Quat& a, const Quat& b);
bool operator==(const Quat& q1, const Quat& q2);

bool almost_equal(const Vec2& v1, const Vec2& v2);
bool almost_equal(const Vec3& v1, const Vec3& v2);
bool almost_equal(const Vec4& v1, const Vec4& v2);

Mat4 mat4_identity();
Mat4 mat4_inverse(const Mat4& m);
Mat4 mat4_from_rotation_and_translation(const Quat& q, const Vec3& t);
Vec3 mat4_right(const Mat4& m);
Vec3 mat4_up(const Mat4& m);

Vec3 cross(const Vec3& v1, const Vec3& v2);
float vec3_len(const Vec3& v);
float vec3_sq_length(const Vec3& v);
float dot(const Vec3& v1, const Vec3& v2);
Vec3 vec3_tangent(const Vec3& v);
Vec3 vec3_bitangent(const Vec3& v);
Vec3 vec3_normalize(const Vec3& v);

static const Vec3 vec3_up = {0, 0, 1};
static const Vec3 vec3_forward = {0, 1, 0};
static const Vec3 vec3_zero = {0, 0, 0};

float dot(const Vec4& v1, const Vec4& v2);

Quat quat_rotate_x(const Quat& q, float rads);
Quat quat_rotate_y(const Quat& q, float rads);
Quat quat_rotate_z(const Quat& q, float rads);
Quat quat_from_axis_angle(const Vec3& axis, float angle);
Quat quat_identity();
Quat quat_normalize(const Quat& q);
Quat quat_look_at(const Vec3& source, const Vec3& dest);
Quat quat_conjugate(const Quat& q);
Vec3 quat_transform_vec3(const Quat&q, const Vec3& v);

bool almost_equal(float f1, float f2);
int mini(int i1, int i2);
float minf(float f1, float f2);