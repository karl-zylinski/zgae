#pragma once
#include "vertex.h"

struct Mesh
{
    Vertex* vertices;
    unsigned* indices;
    unsigned num_vertices;
    unsigned num_indices;
};