#pragma once
#include "mesh_types.h"

typedef struct ObjLoadResult
{
    bool ok;
    Mesh mesh;
} ObjLoadResult;

typedef struct ObjLoadVerticesResult
{
    bool ok;
    Vec3* vertices;
    u32 vertices_num;
} ObjLoadVerticesResult;