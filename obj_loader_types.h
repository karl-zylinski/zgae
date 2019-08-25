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
    size_t vertices_num;
} ObjLoadVerticesResult;