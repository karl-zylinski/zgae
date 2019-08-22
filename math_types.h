#pragma once

typedef struct Vec2u
{
    u32 x, y;
} Vec2u;

typedef struct Vec3
{
    f32 x, y, z;
} Vec3;

typedef struct Vec4
{
    f32 x, y, z, w;
} Vec4;

typedef struct Color
{
    f32 r, g, b, a;
} Color;

typedef struct Quat
{
    f32 x, y, z, w;
} Quat;

typedef struct Mat4
{
    Vec4 x, y, z, w;
} Mat4;
