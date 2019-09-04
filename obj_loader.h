#pragma once
#include "mesh.h"

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

ObjLoadResult obj_load(char* filename);
ObjLoadVerticesResult obj_load_vertices(char* filename);
