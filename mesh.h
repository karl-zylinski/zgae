#pragma once
#include "dynamic_array.h"
#include "vertex.h"

struct Mesh
{
    Vertex* vertices;
    unsigned* indices;
    unsigned num_vertices;
    unsigned num_indices;
};