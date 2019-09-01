#pragma once

#include "math_types.h"

struct GjkShape
{
    Vec3* vertices;
    u32 vertices_num;
};

struct GjkEpaSolution
{
    bool colliding;
    Vec3 solution;
};