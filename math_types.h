#pragma once

typedef struct vec2u_t
{
    uint32_t x, y;
} vec2u_t;

typedef struct vec3_t
{
    float x, y, z;
} vec3_t;

typedef struct vec4_t
{
    float x, y, z, w;
} vec4_t;

typedef struct color_t
{
    float r, g, b, a;
} color_t;

typedef struct
{
    float x, y, z, w;
} quat_t;

typedef struct mat4_t
{
    vec4_t x, y, z, w;
} mat4_t;
