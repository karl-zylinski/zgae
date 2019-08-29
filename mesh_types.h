#pragma once
#include "math_types.h"

struct MeshVertex
{
    Vec3 position;
    Vec3 normal;
    Vec4 color;
    Vec2 texcoord;
};

typedef u16 MeshIndex;

struct Mesh
{
    MeshVertex* vertices;
    MeshIndex* indices;
    u32 vertices_num;
    u32 indices_num;
};