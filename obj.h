#pragma once
#include "mesh.h"

struct Vec3;

struct LoadedMesh
{
    bool valid;
    Mesh mesh;
};

struct LoadedVertices
{
    bool valid;
    Vec3* vertices;
    size_t num_vertices;
};

LoadedMesh obj_load(const char* filename);
LoadedVertices obj_load_only_vertices(const char* filename);
