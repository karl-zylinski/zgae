#pragma once
#include "math.h"
#include "render_resource.h"

struct RenderObject
{
    unsigned long long id;
    Matrix4x4 world_transform;
    RRHandle geometry_handle;
    bool is_light;
};

struct RenderObjectResource
{
    RenderObject ro;
    bool used;
};
