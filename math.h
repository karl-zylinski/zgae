#pragma once

struct Vector2
{
    float x, y;
};

struct Vector2i
{
    int x, y;
};

struct Vector3
{
    float x, y, z;
};

struct Vector4
{
    float x, y, z, w;
};

struct Matrix4x4
{
    Vector4 x, y, z, w;
};

struct Quaternion
{
    float x, y, z, w;
};

static const float PI = 3.1415926535897932f;

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);
void operator+=(Vector2& v1, const Vector2& v2);
Vector2 operator+(const Vector2& v1, const Vector2& v2);
void operator+=(Vector2i& v1, const Vector2i& v2);
Vector2i operator+(const Vector2i& v1, const Vector2i& v2);
void operator+=(Vector3& v1, const Vector3& v2);
Vector3 operator-(const Vector3& v);
Vector3 operator+(const Vector3& v1, const Vector3& v2);
Vector3 operator-(const Vector3& v1, const Vector3& v2);
Vector3 operator*(const Vector3& v, float s);
Vector3 operator*(const Vector3& v1, const Vector3& v2);
bool operator==(const Vector3& v1, const Vector3& v2);
void operator+=(Vector4& v1, const Vector4& v2);
Vector4 operator+(const Vector4& v1, const Vector4& v2);
Vector4 operator*(const Vector4& v, float s);
Vector4 operator*(const Vector4& v1, const Vector4& v2);
Vector4 operator*(const Vector4& v, const Matrix4x4& m);
Matrix4x4 operator*(const Matrix4x4& m, float s);
Vector4 operator-(const Vector4& v1, const Vector4& v2);
Quaternion operator*(const Quaternion& a, const Quaternion& b);
bool operator==(const Quaternion& q1, const Quaternion& q2);

bool almost_equal(const Vector2& v1, const Vector2& v2);
bool almost_equal(const Vector3& v1, const Vector3& v2);
bool almost_equal(const Vector4& v1, const Vector4& v2);

Matrix4x4 matrix4x4_identity();
Matrix4x4 matrix4x4_inverse(const Matrix4x4& m);
Matrix4x4 matrix4x4_from_rotation_and_translation(const Quaternion& q, const Vector3& t);
Vector3 matrix4x4_right(const Matrix4x4& m);
Vector3 matrix4x4_up(const Matrix4x4& m);

Vector3 vector3_cross(const Vector3& v1, const Vector3& v2);
float vector3_length(const Vector3& v);
float vector3_squared_length(const Vector3& v);
float vector3_dot(const Vector3& v1, const Vector3& v2);
Vector3 vector3_tangent(const Vector3& v);
Vector3 vector3_bitangent(const Vector3& v);
Vector3 vector3_normalize(const Vector3& v);

static const Vector3 vector3_up = {0, 1, 0};
static const Vector3 vector3_forward = {0, 0, 1};
static const Vector3 vector3_zero = {0, 0, 0};
static const Vector3 vector3_lookdir = {0, 0, -1};

float vector4_dot(const Vector4& v1, const Vector4& v2);

Quaternion quaternion_rotate_x(const Quaternion& q, float rads);
Quaternion quaternion_rotate_y(const Quaternion& q, float rads);
Quaternion quaternion_rotate_z(const Quaternion& q, float rads);
Quaternion quaternion_from_axis_angle(const Vector3& axis, float angle);
Quaternion quaternion_identity();
Quaternion quaternion_normalize(const Quaternion& q);
Quaternion quaternion_look_at(const Vector3& source, const Vector3& dest);
Quaternion quaternion_conjugate(const Quaternion& q);
Vector3 quaternion_transform_vector3(const Quaternion&q, const Vector3& v);

bool almost_equal(float f1, float f2);
int mini(int i1, int i2);
float minf(float f1, float f2);