#pragma once
#include "math_types.h"

typedef struct GeometryVertex {
    Vec3 position;
    Vec3 normal;
    Vec4 color;
    Vec2 texcoord;
} GeometryVertex;

typedef u16 GeometryIndex;

typedef struct Mesh {
    GeometryVertex* vertices;
    GeometryIndex* indices;
    u32 vertices_num;
    u32 indices_num;
} Mesh;