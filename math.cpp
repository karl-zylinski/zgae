#include "math.h"
#include <math.h>
#include "log.h"

Mat4 mat4_create_projection_matrix(f32 bb_width, f32 bb_height)
{
    f32 near_plane = 0.01f;
    f32 far_plane = 1000.0f;
    f32 fov = 75.0f;
    f32 aspect = bb_width / bb_height;
    f32 y_scale = 1.0f / tanf((PI / 180.0f) * fov / 2);
    f32 x_scale = y_scale / aspect;
    Mat4 proj = {
        {x_scale, 0, 0, 0},
        {0, 0, far_plane/(far_plane-near_plane), 1},
        {0, -y_scale, 0, 0},
        {0, 0, (-far_plane * near_plane) / (far_plane - near_plane), 0}
    };
    return proj;
}

Mat4 mat4_identity()
{
    Mat4 i = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };

    return i;
}

Mat4 mat4_from_rotation_and_translation(const Quat& q, const Vec3& t)
{
    f32 x = q.x, y = q.y, z = q.z, w = q.w,
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

    Mat4 out = {};
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

Mat4 operator*(const Mat4& m1, const Mat4& m2)
{
    return {
        {
            m1.x.x * m2.x.x + m1.x.y * m2.y.x + m1.x.z * m2.z.x + m1.x.w * m2.w.x,
            m1.x.x * m2.x.y + m1.x.y * m2.y.y + m1.x.z * m2.z.y + m1.x.w * m2.w.y,
            m1.x.x * m2.x.z + m1.x.y * m2.y.z + m1.x.z * m2.z.z + m1.x.w * m2.w.z,
            m1.x.x * m2.x.w + m1.x.y * m2.y.w + m1.x.z * m2.z.w + m1.x.w * m2.w.w
        },
        {
            m1.y.x * m2.x.x + m1.y.y * m2.y.x + m1.y.z * m2.z.x + m1.y.w * m2.w.x,
            m1.y.x * m2.x.y + m1.y.y * m2.y.y + m1.y.z * m2.z.y + m1.y.w * m2.w.y,
            m1.y.x * m2.x.z + m1.y.y * m2.y.z + m1.y.z * m2.z.z + m1.y.w * m2.w.z,
            m1.y.x * m2.x.w + m1.y.y * m2.y.w + m1.y.z * m2.z.w + m1.y.w * m2.w.w
        },
        {
            m1.z.x * m2.x.x + m1.z.y * m2.y.x + m1.z.z * m2.z.x + m1.z.w * m2.w.x,
            m1.z.x * m2.x.y + m1.z.y * m2.y.y + m1.z.z * m2.z.y + m1.z.w * m2.w.y,
            m1.z.x * m2.x.z + m1.z.y * m2.y.z + m1.z.z * m2.z.z + m1.z.w * m2.w.z,
            m1.z.x * m2.x.w + m1.z.y * m2.y.w + m1.z.z * m2.z.w + m1.z.w * m2.w.w
        },
        {
            m1.w.x * m2.x.x + m1.w.y * m2.y.x + m1.w.z * m2.z.x + m1.w.w * m2.w.x,
            m1.w.x * m2.x.y + m1.w.y * m2.y.y + m1.w.z * m2.z.y + m1.w.w * m2.w.y,
            m1.w.x * m2.x.z + m1.w.y * m2.y.z + m1.w.z * m2.z.z + m1.w.w * m2.w.z,
            m1.w.x * m2.x.w + m1.w.y * m2.y.w + m1.w.z * m2.z.w + m1.w.w * m2.w.w
        }
    };
}

Mat4 inverse(const Mat4& m)
{
    const float* a = &m.x.x;
    f32 a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3],
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
    f32 det = 1.0f / (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

    Mat4 result;
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

f32 dot(const Vec3& v1, const Vec3& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

f32 len(const Vec3& v)
{
    return sqrtf(dot(v, v));
}

Vec3 normalize(const Vec3& v)
{
    if (v == vec3_zero)
        return v;

    return v * (1/len(v));
}

Vec3 operator-(const Vec3& v)
{
    return {-v.x, -v.y, -v.z};
}

Vec3 operator-(const Vec3& v1, const Vec3& v2)
{
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

Vec3 operator*(const Vec3& v, f32 s)
{
    return {v.x * s, v.y * s, v.z * s};
}

Vec3 cross(const Vec3& v1, const Vec3& v2)
{
    return  {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}

Vec3 operator+(const Vec3& v1, const Vec3& v2)
{
    return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

void operator+=(Vec3& v1, const Vec3& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
}

void operator*=(Vec3& v1, f32 s)
{
    v1 = v1 * s;
}

bool operator==(const Vec3& v1, const Vec3& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

Vec3 operator*(f32 s, const Vec3& v)
{
    return v * s;
}

void operator-=(Vec3& v1, const Vec3& v2)
{
    v1 = v1 - v2;
}

Vec3 operator/(const Vec3& v, f32 s)
{
    check(s != 0, "Trying to divide by zero");
    return {v.x / s, v.y / s, v.z / s};
}

Vec3 project(const Vec3& v, const Vec3& on)
{
    let unit_dir = on*(1/len(on));
    return dot(v, unit_dir) * unit_dir;
}

Vec2 operator*(const Vec2i& v, f32 s)
{
    return { v.x * s, v.y * s };
}

bool almost_eql(f32 f1, f32 f2, f32 thres)
{
    return fabs(f2 - f1) < thres;
}

bool almost_eql(const Vec2& v1, const Vec2& v2, f32 thres)
{
    return almost_eql(v1.x, v2.x, thres) && almost_eql(v1.y, v2.y, thres);
}

bool almost_eql(const Vec3& v1, const Vec3& v2, f32 thres)
{
    return almost_eql(v1.x, v2.x, thres) && almost_eql(v1.y, v2.y, thres) && almost_eql(v1.z, v2.z, thres);
}

bool almost_eql(const Vec4& v1, const Vec4& v2, f32 thres)
{
    return almost_eql(v1.x, v2.x, thres) && almost_eql(v1.y, v2.y, thres) && almost_eql(v1.z, v2.z, thres) && almost_eql(v1.w, v2.w, thres);
}

Quat quat_identity()
{
    return {0, 0, 0, 1};
}

Quat operator*(const Quat& a, const Quat& b)
{
    return
    {
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

void operator*=(Quat& a, const Quat& b)
{
    a = a * b;
}

Quat quat_rotate_x(const Quat& q, float rads)
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

Quat quat_rotate_y(const Quat& q, float rads)
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

Quat quat_rotate_z(const Quat& q, float rads)
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

Quat quat_from_axis_angle(const Vec3& axis, float angle)
{
    float half_angle = angle * 0.5f;
    float s = sin(half_angle);
    return normalize({
        axis.x * s,
        axis.y * s,
        axis.z * s,
        cos(half_angle)
    });
}

Quat normalize(const Quat& q)
{
    float len = sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    check(len != 0, "Trying to normalize zero length Quat");
    return
    {
        q.x / len,
        q.y / len,
        q.z / len,
        q.w / len
    };
}
    
Vec3 rotate_vec3(const Quat& q, const Vec3& v)
{
    const Vec3 qv = {q.x, q.y, q.z};
    const Vec3 uv = cross(qv, v);
    const Vec3 uuv = cross(qv, uv);
    return v + ((uv * q.w) + uuv) * 2.0f;
}

f32 clamp(f32 v, f32 minv, f32 maxv)
{
    return fmax(minv, fmin(maxv, v));
}