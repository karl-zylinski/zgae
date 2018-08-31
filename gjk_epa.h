#pragma once
#include "math.h"

struct GJKShape
{
    Vec3* vertices;
    size_t num_vertices;
    Vec3 position;
};

struct GJKEPASolution
{
    bool colliding;
    Vec3 solution;
};

bool gjk_intersect(const GJKShape& s1, const GJKShape& s2);
GJKEPASolution gjk_epa_intersect_and_solve(const GJKShape& s1, const GJKShape& s2);