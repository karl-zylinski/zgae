#pragma once

struct Vec2
{
    f32 x, y;
};

struct Vec3
{
    f32 x, y, z;
};

struct Vec4
{
    f32 x, y, z, w;
};

struct Color
{
    f32 r, g, b, a;
};

struct Vec2u
{
    u32 x, y;
};

struct Quat
{
    f32 x, y, z, w;
};

struct Mat4
{
    Vec4 x, y, z, w;
};
