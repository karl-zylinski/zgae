#pragma once
#include "mesh_types.h"

struct ObjLoadResult
{
    bool ok;
    Mesh mesh;
};

struct ObjLoadVerticesResult
{
    bool ok;
    Vec3* vertices;
    u32 vertices_num;
};