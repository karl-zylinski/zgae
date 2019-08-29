#pragma once

#include "math_types.h"

struct GjkShape
{
    Vec3* vertices;
    u32 vertices_num;
    Vec3 position;
};

struct GjkEpaSolution
{
    bool colliding;
    Vec3 solution;
};