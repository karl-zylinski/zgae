#pragma once

struct Vector3;

struct GJKShape
{
    Vector3* vertices;
    size_t num_vertices;
};

bool gjk_intersect(const GJKShape& s1, const GJKShape& s2);