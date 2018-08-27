#pragma once
#include "mesh.h"

struct LoadedMesh
{
    bool valid;
    Mesh mesh;
};

LoadedMesh obj_load(const char* filename);
