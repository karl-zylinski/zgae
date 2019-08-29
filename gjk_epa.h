#pragma once
#include "math.h"

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

bool gjk_intersect(GjkShape* s1, GjkShape* s2);
GjkEpaSolution gjk_epa_intersect_and_solve(GjkShape* s1, GjkShape* s2);