#pragma once
#include "math_types.h"

typedef struct MeshVertex {
    Vec3 position;
    Vec3 normal;
    Vec4 color;
    Vec2 texcoord;
} MeshVertex;

typedef u16 MeshIndex;

typedef struct Mesh {
    MeshVertex* vertices;
    MeshIndex* indices;
    u32 vertices_num;
    u32 indices_num;
} Mesh;