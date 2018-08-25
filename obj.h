#pragma once
#include "mesh.h"

struct Allocator;

struct LoadedMesh
{
    bool valid;
    Mesh mesh;
};

LoadedMesh obj_load(Allocator* alloc, const char* filename);
