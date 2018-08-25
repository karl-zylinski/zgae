#include "math.h"
#include <cmath>

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2)
{
    return
    {
        m1.x.x * m2.x.x + m1.x.y * m2.y.x + m1.x.z * m2.z.x + m1.x.w * m2.w.x,
        m1.x.x * m2.x.y + m1.x.y * m2.y.y + m1.x.z * m2.z.y + m1.x.w * m2.w.y,
        m1.x.x * m2.x.z + m1.x.y * m2.y.z + m1.x.z * m2.z.z + m1.x.w * m2.w.z,
        m1.x.x * m2.x.w + m1.x.y * m2.y.w + m1.x.z * m2.z.w + m1.x.w * m2.w.w,

        m1.y.x * m2.x.x + m1.y.y * m2.y.x + m1.y.z * m2.z.x + m1.y.w * m2.w.x,
        m1.y.x * m2.x.y + m1.y.y * m2.y.y + m1.y.z * m2.z.y + m1.y.w * m2.w.y,
        m1.y.x * m2.x.z + m1.y.y * m2.y.z + m1.y.z * m2.z.z + m1.y.w * m2.w.z,
        m1.y.x * m2.x.w + m1.y.y * m2.y.w + m1.y.z * m2.z.w + m1.y.w * m2.w.w,

        m1.z.x * m2.x.x + m1.z.y * m2.y.x + m1.z.z * m2.z.x + m1.z.w * m2.w.x,
        m1.z.x * m2.x.y + m1.z.y * m2.y.y + m1.z.z * m2.z.y + m1.z.w * m2.w.y,
        m1.z.x * m2.x.z + m1.z.y * m2.y.z + m1.z.z * m2.z.z + m1.z.w * m2.w.z,
        m1.z.x * m2.x.w + m1.z.y * m2.y.w + m1.z.z * m2.z.w + m1.z.w * m2.w.w,

        m1.w.x * m2.x.x + m1.w.y * m2.y.x + m1.w.z * m2.z.x + m1.w.w * m2.w.x,
        m1.w.x * m2.x.y + m1.w.y * m2.y.y + m1.w.z * m2.z.y + m1.w.w * m2.w.y,
        m1.w.x * m2.x.z + m1.w.y * m2.y.z + m1.w.z * m2.z.z + m1.w.w * m2.w.z,
        m1.w.x * m2.x.w + m1.w.y * m2.y.w + m1.w.z * m2.z.w + m1.w.w * m2.w.w
    };
}

void operator+=(Vector2& v1, const Vector2& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
}

Vector2 operator+(const Vector2& v1, const Vector2& v2)
{
    return {v1.x + v2.x, v1.y + v2.y};
}

void operator+=(Vector2i& v1, const Vector2i& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
}

Vector2i operator+(const Vector2i& v1, const Vector2i& v2)
{
    return {v1.x + v2.x, v1.y + v2.y};
}

void operator+=(Vector3& v1, const Vector3& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
}

Vector3 operator-(const Vector3& v)
{
    return {-v.x, -v.y, -v.z};
}

Vector3 operator+(const Vector3& v1, const Vector3& v2)
{
    return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

Vector3 operator-(const Vector3& v1, const Vector3& v2)
{
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

Vector3 operator*(const Vector3& v, float s)
{
    return {v.x * s, v.y * s, v.z * s};
}

Vector3 operator*(const Vector3& v1, const Vector3& v2)
{
    return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

void operator+=(Vector4& v1, const Vector4& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    v1.w += v2.w;
}

Vector4 operator+(const Vector4& v1, const Vector4& v2)
{
    return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w + v2.w};
}

Vector4 operator*(const Vector4& v, float s)
{
    return {v.x * s, v.y * s, v.z * s, v.w * s};
}

Vector4 operator*(const Vector4& v1, const Vector4& v2)
{
    return {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w};
}

Vector4 operator*(const Vector4& v, const Matrix4x4& m)
{
    return
    {
        v.x * m.x.x + v.x * m.x.y + v.x * m.x.z + v.x * m.x.w,
        v.y * m.y.x + v.y * m.y.y + v.y * m.y.z + v.y * m.y.w,
        v.z * m.z.x + v.z * m.z.y + v.z * m.z.z + v.z * m.z.w,
        v.x * m.w.x + v.w * m.w.y + v.w * m.w.z + v.w * m.w.w
    };
}

Matrix4x4 operator*(const Matrix4x4& m, float s)
{
    return
    {
        s * m.x.x, s * m.x.y, s * m.x.z, s * m.x.w,
        s * m.y.x, s * m.y.y, s * m.y.z, s * m.y.w,
        s * m.z.x, s * m.z.y, s * m.z.z, s * m.z.w,
        s * m.w.x, s * m.w.y, s * m.w.z, s * m.w.w
    };
}

Vector4 operator-(const Vector4& v1, const Vector4& v2)
{
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w};
}

Quaternion operator*(const Quaternion& a, const Quaternion& b)
{
    return
    {
        a.x * b.w + a.w * b.x + a.y * b.z - a.z * b.y,
        a.y * b.w + a.w * b.y + a.z * b.x - a.x * b.z,
        a.z * b.w + a.w * b.z + a.x * b.y - a.y * b.x,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

bool almost_equal(const Vector2& v1, const Vector2& v2)
{
    return almost_equal(v1.x, v2.x) && almost_equal(v1.y, v2.y);
}

bool almost_equal(const Vector3& v1, const Vector3& v2)
{
    return almost_equal(v1.x, v2.x) && almost_equal(v1.y, v2.y) && almost_equal(v1.z, v2.z);
}

Matrix4x4 matrix4x4_identity()
{
    return
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

Matrix4x4 matrix4x4_inverse(const Matrix4x4& m)
{
    const float* a = &m.x.x;
    float a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3],
        a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7],
        a20 = a[8], a21 = a[9], a22 = a[10], a23 = a[11],
        a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15],

        b00 = a00 * a11 - a01 * a10,
        b01 = a00 * a12 - a02 * a10,
        b02 = a00 * a13 - a03 * a10,
        b03 = a01 * a12 - a02 * a11,
        b04 = a01 * a13 - a03 * a11,
        b05 = a02 * a13 - a03 * a12,
        b06 = a20 * a31 - a21 * a30,
        b07 = a20 * a32 - a22 * a30,
        b08 = a20 * a33 - a23 * a30,
        b09 = a21 * a32 - a22 * a31,
        b10 = a21 * a33 - a23 * a31,
        b11 = a22 * a33 - a23 * a32;

    // Calculate the determinant
    float det = 1.0f / (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

    Matrix4x4 result;
    result.x.x = (a11 * b11 - a12 * b10 + a13 * b09) * det;
    result.x.y = (a02 * b10 - a01 * b11 - a03 * b09) * det;
    result.x.z = (a31 * b05 - a32 * b04 + a33 * b03) * det;
    result.x.w = (a22 * b04 - a21 * b05 - a23 * b03) * det;
    result.y.x = (a12 * b08 - a10 * b11 - a13 * b07) * det;
    result.y.y = (a00 * b11 - a02 * b08 + a03 * b07) * det;
    result.y.z = (a32 * b02 - a30 * b05 - a33 * b01) * det;
    result.y.w = (a20 * b05 - a22 * b02 + a23 * b01) * det;
    result.z.x = (a10 * b10 - a11 * b08 + a13 * b06) * det;
    result.z.y = (a01 * b08 - a00 * b10 - a03 * b06) * det;
    result.z.z = (a30 * b04 - a31 * b02 + a33 * b00) * det;
    result.z.w = (a21 * b02 - a20 * b04 - a23 * b00) * det;
    result.w.x = (a11 * b07 - a10 * b09 - a12 * b06) * det;
    result.w.y = (a00 * b09 - a01 * b07 + a02 * b06) * det;
    result.w.z = (a31 * b01 - a30 * b03 - a32 * b00) * det;
    result.w.w = (a20 * b03 - a21 * b01 + a22 * b00) * det;
    return result;
}

Matrix4x4 matrix4x4_from_rotation_and_translation(const Quaternion& q, const Vector3& t)
{
    const float x = q.x, y = q.y, z = q.z, w = q.w,
        x2 = x + x,
        y2 = y + y,
        z2 = z + z,

        xx = x * x2,
        xy = x * y2,
        xz = x * z2,
        yy = y * y2,
        yz = y * z2,
        zz = z * z2,
        wx = w * x2,
        wy = w * y2,
        wz = w * z2;

    Matrix4x4 out = {};
    out.x.x = 1 - (yy + zz);
    out.x.y = xy + wz;
    out.x.z = xz - wy;
    out.x.w = 0;
    out.y.x = xy - wz;
    out.y.y = 1 - (xx + zz);
    out.y.z = yz + wx;
    out.y.w = 0;
    out.z.x = xz + wy;
    out.z.y = yz - wx;
    out.z.z = 1 - (xx + yy);
    out.z.w = 0;
    out.w.x = t.x;
    out.w.y = t.y;
    out.w.z = t.z;
    out.w.w = 1;
    return out;
}

Vector3 matrix4x4_right(const Matrix4x4& m)
{
    return {m.x.x, m.x.y, m.x.z};
}

Vector3 matrix4x4_up(const Matrix4x4& m)
{
    return {m.y.x, m.y.y, m.y.z};
}

Vector3 vector3_cross(const Vector3& v1, const Vector3& v2)
{
    return 
    {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}

float vector3_length(const Vector3& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vector3_squared_length(const Vector3& v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

float vector3_dot(const Vector3& v1, const Vector3& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 vector3_normalize(const Vector3& v)
{
    float len = vector3_length(v);
    return
    {
        v.x / len,
        v.y / len,
        v.z / len
    };
}

Vector3 vector3_tangent(const Vector3& v)
{
    Vector3 c1 = vector3_cross(v, {0.0, 0.0, 1.0});
    Vector3 c2 = vector3_cross(v, {0.0, 1.0, 0.0});
    return vector3_normalize(vector3_squared_length(c1) > vector3_squared_length(c2) ? c1 : c2);
}

Vector3 vector3_bitangent(const Vector3& v)
{
    return vector3_normalize(vector3_cross(vector3_tangent(v), v));
}

float vector4_dot(const Vector4& v1, const Vector4& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

Quaternion quaternion_rotate_x(const Quaternion& q, float rads)
{
    float adjusted_rads = rads * 0.5f; 
    float bx = sin(adjusted_rads);
    float bw = cos(adjusted_rads);
    return
    {
        q.x * bw + q.w * bx,
        q.y * bw + q.z * bx,
        q.z * bw - q.y * bx,
        q.w * bw - q.x * bx
    };
}

Quaternion quaternion_rotate_y(const Quaternion& q, float rads)
{
    float adjusted_rads = rads * 0.5f;
    float by = sin(adjusted_rads);
    float bw = cos(adjusted_rads);
    return
    {
        q.x * bw - q.z * by,
        q.y * bw + q.w * by,
        q.z * bw + q.x * by,
        q.w * bw - q.y * by
    };
}

Quaternion quaternion_rotate_z(const Quaternion& q, float rads)
{
    float adjusted_rads = rads * 0.5f;
    float bz = sin(adjusted_rads);
    float bw = cos(adjusted_rads);
    return
    {
        q.x * bw + q.y * bz,
        q.y * bw - q.x * bz,
        q.z * bw + q.w * bz,
        q.w * bw - q.z * bz
    };
}

Quaternion quaternion_from_axis_angle(const Vector3& axis, float angle)
{
    float half_angle = angle * 0.5f;
    float s = sin(half_angle);
    return
    quaternion_normalize({
        axis.x * s,
        axis.y * s,
        axis.z * s,
        cos(half_angle)
    });
}

Quaternion quaternion_identity()
{
    return {0, 0, 0, 1};
}

Quaternion quaternion_normalize(const Quaternion& q)
{
    float len = sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    return
    {
        q.x / len,
        q.y / len,
        q.z / len,
        q.w / len
    };
}

Quaternion quaternion_look_at(const Vector3& source, const Vector3& dest)
{
    Vector3 source_to_dest = vector3_normalize(dest - source);
    float dot = vector3_dot(vector3_forward, source_to_dest);

    if (fabs(dot - (-1.0f)) < SmallNumber)
    {
        return quaternion_from_axis_angle(vector3_up, PI);
    }

    if (fabs(dot - ( 1.0f)) < SmallNumber)
    {
        return quaternion_identity();
    }

    float rot_angle = acos(dot);
    Vector3 rot_axis = vector3_normalize(vector3_cross(vector3_forward, source_to_dest));
    return quaternion_from_axis_angle(rot_axis, rot_angle);
}

Quaternion quaternion_conjugate(const Quaternion& q)
{
    return
    {
        -q.x,
        -q.y,
        -q.z,
        q.w
    };
}

Vector3 quaternion_transform_vector3(const Quaternion& q, const Vector3& v)
{
    const Vector3 qv = {q.x, q.y, q.z};
    const Vector3 uv = vector3_cross(qv, v);
    const Vector3 uuv = vector3_cross(qv, uv);
    return v + ((uv * q.w) + uuv) * 2.0f;
}

bool almost_equal(float f1, float f2)
{
    return fabs(f2 - f1) < SmallNumber;
}
