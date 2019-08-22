#pragma once
#include "geometry_types.h"

fwd_struct(Vec3);

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