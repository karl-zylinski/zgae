#pragma once

struct Vertex;

struct Mesh
{
    Vertex* vertices;
    unsigned* indices;
    unsigned num_vertices;
    unsigned num_indices;
};