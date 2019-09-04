#pragma once
#include "math.h"

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

bool gjk_intersect(const GjkShape& s1, const GjkShape& s2);
GjkEpaSolution gjk_epa_intersect_and_solve(const GjkShape& s1, const GjkShape& s2);
