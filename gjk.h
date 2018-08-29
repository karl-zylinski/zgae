#pragma once

struct Vec3;

struct GJKShape
{
    Vec3* vertices;
    size_t num_vertices;
};

bool gjk_intersect(const GJKShape& s1, const GJKShape& s2);