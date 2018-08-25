#pragma once
#include "dynamic_array.h"
#include "vertex.h"

struct Mesh
{
    DynamicArray<Vertex> vertices;
    DynamicArray<unsigned> indices;
};

inline Mesh mesh_create(Allocator* alloc)
{
    Mesh m;
    m.vertices = dynamic_array_create<Vertex>(alloc);
    m.indices = dynamic_array_create<unsigned>(alloc);
    return m;
}

inline void mesh_destroy(Mesh* m)
{
    dynamic_array_destroy(&m->vertices);
    dynamic_array_destroy(&m->indices);
}
